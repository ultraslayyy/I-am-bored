import fs from 'fs-extra';
import path from 'path';

export async function removePackage(packageName: string, projectRoot = process.cwd(), modulesPath = path.join(projectRoot, 'node_modules')) {
    const isScoped = packageName.startsWith('@');
    const packagePath = isScoped ? path.join(modulesPath, ...packageName.split('/')) : path.join(modulesPath, packageName);
    const packageJsonPath = path.join(projectRoot, 'package.json');
    const lockfilePath = path.join(projectRoot, 'vex-lock.json');

    try {
        await fs.remove(packagePath);
        
        const packageJson = await fs.readJson(packageJsonPath);
        const sections = ['dependencies', 'devDependencies', 'peerDependencies', 'optionalDependencies'];
        let updatedPackageJson = false;

        for (const section of sections) {
            if (packageJson[section] && packageJson[section][packageName]) {
                delete packageJson[section][packageName];
                updatedPackageJson = true;
            }
        }

        if (updatedPackageJson) {
            await fs.writeJson(packageJsonPath, packageJson, { spaces: 2 });
        }

        if (await fs.pathExists(lockfilePath)) {
            const lockJson = await fs.readJson(lockfilePath);
            if (lockJson[packageName]) {
                delete lockJson[packageName];
                await fs.writeJson(lockfilePath, lockJson, { spaces: 2});
            }
        }

        console.log(`Removed ${packageName}`);
    } catch (err) {
        console.error(`Failed to removed ${packageName}:`, err);
    }
}