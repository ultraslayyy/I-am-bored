import fs from 'fs';

export interface vexLockfile {
    [pkg: string]: {
        version: string;
        resolved: string;
        integrity: string;
        dependencies?: string;
    }
}

const lockfilePaths = {
    'vex': 'vex.lock.json',
    'npm': 'package-lock.json'
};

export function updateLockfile(pkg: string, version: string, metadata: any, type: 'npm' | 'vex' = 'npm') {
    const LOCKFILE_PATH = lockfilePaths[type];

    if (type === 'vex') {
        let data: vexLockfile = {}

        if (fs.existsSync(LOCKFILE_PATH)) {
            data = JSON.parse(fs.readFileSync(LOCKFILE_PATH, 'utf-8'));
        }

        data[pkg] = {
            version,
            resolved: metadata.dist.tarball,
            integrity: metadata.dist.integrity,
            dependencies: metadata.dependencies
        }

        fs.writeFileSync(LOCKFILE_PATH, JSON.stringify(data, null, 2));
    } else if (type === 'npm') {
        let data: tempNpmLockfile = {
            packages: {}
        };
        let packageJson: PackageJson;

        if (fs.existsSync(LOCKFILE_PATH)) {
            data = JSON.parse(fs.readFileSync(LOCKFILE_PATH, 'utf-8'));
        }

        if (fs.existsSync('package.json')) {
            packageJson = JSON.parse(fs.readFileSync('package.json', 'utf-8'));
        } else return;

        data.name = packageJson.name;
        data.version = packageJson.version;
        data.lockfileVersion = 3;

        data.packages[""] = {
            name: packageJson.name,
            version: packageJson.version,
            license: packageJson.license,
            dependencies: packageJson.dependencies,
            bin: packageJson.bin,
            devDependencies: packageJson.devDependencies
        }

        data.packages[pkg] = {
            version,
            resolved: metadata.dist.tarball,
            integrity: metadata.dist.integrity,
            cpu: metadata.cpu,
            license: metadata.license,
            dependencies: metadata.dependencies,
            bin: metadata.bin,
            engines: metadata.engines,
            funding: metadata.funding,
            optionalDependencies: metadata.optionalDependencies,
            peerDependencies: metadata.peerDependencies,
            peerDependenciesMeta: metadata.peerDependenciesMeta
        }

        fs.writeFileSync(LOCKFILE_PATH, JSON.stringify(data, null, 2));
    }
}

interface npmLockfile {
    name: string;
    version: string;
    lockfileVersion: number;
    requires: boolean;
    packages: {
        "": {
            name: string;
            version: string;
            license: string;
            dependencies?: {
                [dep: string]: string;
            }
            bin?: {
                [bin: string]: string;
            }
            devDependencies?: {
                [devDep: string]: string;
            }
        }
    } & {
        [pkg: string]: {
            version: string;
            resolved: string;
            integrity: string;
            cpu?: string[];
            dev?: boolean;
            license: string;
            workspaces?: string[];
            optional?: boolean;
            os?: string[];
            dependencies?: {
                [dep: string]: string;
            }
            bin?: {
                [bin: string]: string;
            }
            engines?: {
                [engine: string]: string;
            }
            funding?: {
                type?: string;
                url: string;
            }
            optionalDependencies?: {
                [optDep: string]: string;
            }
            peerDependencies?: {
                [peerDep: string]: string;
            }
            peerDependenciesMeta?: {
                [peerDepMeta: string]: {
                    optional: boolean;
                }
            }
        } | {
            version: string;
            resolved: string;
            integrity: string;
            cpu?: string[];
            dev?: boolean;
            funding?: {
                type?: string;
                url: string;
            }[];
            license: string;
            workspaces?: string[];
            optional?: boolean;
            os?: string[];
            dependencies?: {
                [dep: string]: string;
            }
            bin?: {
                [bin: string]: string;
            }
            engines?: {
                [engine: string]: string;
            }
            optionalDependencies?: {
                [optDep: string]: string;
            }
            peerDependencies?: {
                [peerDep: string]: string;
            }
            peerDependenciesMeta?: {
                [peerDepMeta: string]: {
                    optional: boolean;
                }
            }
        }
    }
}

interface PackageJson {
    name: string;
    version: string;
    license: string;
    dependencies?: {
        [dep: string]: string;
    }
    devDependencies?: {
        [devDep: string]: string;
    }
    bin?: {
        [bin: string]: string;
    }
    [entry: string]: any;

}

interface tempNpmLockfile {
    name?: string;
    version?: string;
    lockfileVersion?: number;
    requires?: boolean;
    packages: {
        [pkg: string]: {
            version: string;
            resolved: string;
            integrity: string;
            cpu?: string[];
            dev?: boolean;
            license: string;
            workspaces?: string[];
            optional?: boolean;
            os?: string[];
            dependencies?: {
                [dep: string]: string;
            }
            bin?: {
                [bin: string]: string;
            }
            engines?: {
                [engine: string]: string;
            }
            funding?: {
                type?: string;
                url: string;
            }
            optionalDependencies?: {
                [optDep: string]: string;
            }
            peerDependencies?: {
                [peerDep: string]: string;
            }
            peerDependenciesMeta?: {
                [peerDepMeta: string]: {
                    optional: boolean;
                }
            }
        } | {
            version: string;
            resolved: string;
            integrity: string;
            cpu?: string[];
            dev?: boolean;
            funding?: {
                type?: string;
                url: string;
            }[];
            license: string;
            workspaces?: string[];
            optional?: boolean;
            os?: string[];
            dependencies?: {
                [dep: string]: string;
            }
            bin?: {
                [bin: string]: string;
            }
            engines?: {
                [engine: string]: string;
            }
            optionalDependencies?: {
                [optDep: string]: string;
            }
            peerDependencies?: {
                [peerDep: string]: string;
            }
            peerDependenciesMeta?: {
                [peerDepMeta: string]: {
                    optional: boolean;
                }
            }
        } | {
            name: string;
            version: string;
            license: string;
            dependencies?: {
                [dep: string]: string;
            }
            bin?: {
                [bin: string]: string;
            }
            devDependencies?: {
                [devDep: string]: string;
            }
        }
    }
}