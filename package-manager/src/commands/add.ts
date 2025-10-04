import * as zlib from 'zlib';
import * as tarStream from 'tar-stream'
import { Readable } from 'stream';
import fs from 'fs-extra';
import path from 'path';
import { updateLockfile } from '../lockfile';
import semver from 'semver';

const REGISTRY_URL = 'https://registry.npmjs.org';
const installed = new Set<string>();
let unpackedSize: number = 0;
let packagesNumber: number = 0;

export async function addPackage(packages: Record<DepType, string[]>, isRoot: boolean = true, layer: number = 0, prefixLines: boolean[] = [], first: boolean = true, depFirst: boolean = true) {
    const depTypes: DepType[] = ['dependencies', 'devDependencies', 'peerDependencies'];

    for (const depType of depTypes) {
        const pkgs = packages[depType];
        for (let i = 0; i < pkgs.length; i++) {
            let pkg = pkgs[i];
            let version = 'latest';

            if (pkg.indexOf('@npm:') !== -1) {
                pkg = pkg.split('@npm:')[1];
            }
            
            if (pkg.startsWith('@')) {
                const atIndex = pkg.indexOf('@', 1);
                if (atIndex === -1) {
                    pkg = pkg;
                } else {
                    version = pkg.slice(atIndex + 1);
                    pkg = pkg.slice(0, atIndex);
                }
            } else {
                const atIndex = pkg.indexOf('@');
                if (atIndex === -1) {
                    pkg = pkg;
                } else {
                    version = pkg.slice(atIndex + 1);
                    pkg = pkg.slice(0, atIndex);
                }
            }

            const installKey = `${pkg}@${version}`;
            if (installed.has(installKey)) return;
            installed.add(installKey);

            const isLast = i === pkgs.length - 1;
            //const treePrefix = prefixLines.map(last => (last ? '    ' : '│   ')).join('') + (isRoot && first ? '┌── ' : isLast ? '└── ' : '├── ');
            const treePrefix = prefixLines.map(last => (last ? '    ' : '│   ')).join('') + (isLast ? '└── ' : '├── ');
            if (isRoot && depFirst) console.log(`${depType}`);
            console.log(treePrefix + `${pkg}@${version}`);

            const metaRes = await fetch(`${REGISTRY_URL}/${pkg}`);
            if (!metaRes.ok) {
                console.error(`Failed to fetch metadata for ${pkg}`);
                return;
            }
            const metadata = await metaRes.json();

            let resolvedVersion: string;
            if (version === 'latest') {
                resolvedVersion = metadata['dist-tags'].latest;
            } else if (metadata.versions[version]) {
                resolvedVersion = version;
            } else {
                const allVersions = Object.keys(metadata.versions);
                const max = semver.maxSatisfying(allVersions, version);
                if (!max) {
                    console.error(`No version found for ${pkg}@${version}`);
                    return;
                }
                resolvedVersion = max;
            }

            const versionMeta = metadata.versions[resolvedVersion];
            const tarballUrl = versionMeta.dist.tarball;

            const res = await fetch(tarballUrl);
            if (!res.ok) {
                console.error(`Failed to fetch tarball for ${pkg}@${resolvedVersion}`);
                return;
            }

            if (versionMeta.deprecated) {
                console.warn(`Deprecation Warning: "${versionMeta.deprecated}" - ${pkg}@${resolvedVersion}`);
            }

            await fs.ensureDir('node_modules');
            const extractPath = path.join('node_modules', ...pkg.split('/'));
            await fs.ensureDir(path.dirname(extractPath));
            await fs.ensureDir(extractPath);

            const buffer = Buffer.from(await res.arrayBuffer());
            await extractTarball(buffer, extractPath);

            // if (isRoot) console.log(`Installed ${pkg}@${resolvedVersion} to ${extractPath}`);
            updateLockfile(pkg, resolvedVersion, versionMeta);

            if (isRoot) await updatePackageJson(pkg, resolvedVersion, depType);

            unpackedSize = unpackedSize + (versionMeta.dist.unpackedSize ?? 0);
            packagesNumber = packagesNumber + 1;

            const deps = versionMeta.dependencies || {}
            
            if (Object.keys(deps).length > 0) {
                depFirst = false;

                await addPackage({
                    dependencies: Object.entries(deps).map(([k, v]) => `${k}@${v}`),
                    devDependencies: [],
                    peerDependencies: []
                },
                false,
                layer + 1,
                [...prefixLines, isLast],
                false);
            }

            first = false;
        }
        depFirst = true;
    }
    if (isRoot) {
        console.log('\n\n');
        console.log('Stats');
        console.log(`Size: ${unpackedSize} (${unpackedSize / (1024 * 1024)} MB)`);
        console.log(`Packages: ${packagesNumber}`);
    }
}

export async function extractTarball(buffer: Buffer, extractPath: string) {
    const gunzip = zlib.createGunzip();
    const extract = tarStream.extract();

    let rootPrefix = '';

    extract.on('entry', async (header, stream, next) => {
        if (!rootPrefix) {
            const parts = header.name.split('/');
            rootPrefix = parts[0] + '/';
        }

        if (!header.name.startsWith(rootPrefix)) {
            stream.resume();
            return next();
        }

        const strippedName = header.name.slice(rootPrefix.length);
        const filePath = path.join(extractPath, strippedName);

        if (header.type === 'directory') {
            await fs.ensureDir(filePath);
            stream.resume();
            return next();
        }

        await fs.ensureDir(path.dirname(filePath));
        const writeStream = fs.createWriteStream(filePath);

        stream.pipe(writeStream);
        stream.on('end', next);
        stream.resume();
    });

    Readable.from(buffer).pipe(gunzip).pipe(extract);

    return new Promise<void>((resolve, reject) => {
        extract.on('finish', resolve);
        extract.on('error', reject);
    });
}

type PackageJson = {
    dependencies?: Record<string, string>;
    [key: string]: any;
}

async function updatePackageJson(pkg: string, version: string, depType: DepType) {
    const packageJsonPath = path.join(process.cwd(), 'package.json');

    let packageJson: PackageJson = {}
    if (await fs.pathExists(packageJsonPath)) {
        packageJson = await fs.readJson(packageJsonPath);
    }

    if (!packageJson[depType]) packageJson[depType] = {}

    packageJson[depType][pkg] = `^${version}`;
    await fs.writeJson(packageJsonPath, packageJson, { spaces: 2 });

    // console.log(`Updated package.json with ${pkg}@^${version}`);
}

type DepType = 'dependencies' | 'devDependencies' | 'peerDependencies';
type lockType = 'npm' | 'vex';

export async function parseAddArgs(args: string[]) {
    const dependencies: Record<DepType, string[]> = {
        dependencies: [],
        devDependencies: [],
        peerDependencies: []
    }

    let current: DepType = 'dependencies';
    let lockStyle: lockType = 'npm';

    for (const arg of args) {
        switch (arg) {
            case '--dev':
            case '-D':
                current = 'devDependencies';
                break;
            case '--peer':
            case '-p':
                current = 'peerDependencies';
                break;
            case '--':
                current = 'dependencies';
                break;
            default: dependencies[current].push(arg);
        }
    }

    return dependencies;
}