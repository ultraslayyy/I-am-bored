export function getProjectRoot(projectFilePath: string): string {
    const normalised = projectFilePath.replace(/\\/g, '/');

    const parts = normalised.split('/');
    parts.pop();

    return parts.join('/');
}

export const dirname = (p: string) => {
    const norm = p.replace('\\', '/');
    const parts = norm.split('/');
    parts.pop();
    return parts.join('/');
}