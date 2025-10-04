export const defaultPackageJson = (
    options: {
        name: string,
        version?: string,
        desc?: string,
        main?: string,
        script?: string,
        keywords?: string[],
        author?: string,
        license?: string,
        type?: 'commonjs' | 'module'
    }
) => `{
  "name": "${options.name}",
  "version": "${options.version ?? '1.0.0'}",
  "description": "${options.desc ?? ''}",
  "main": "${options.main ?? 'index.js'}",
  "scripts": {
    "test": "${options.script ?? 'No test script specified'}"
  },
  "keywords": [${options.keywords ?? ''}],
  "author": "${options.author ?? ''}",
  "license": "${options.license ?? 'ISC'}",
  "type": "${options.type ?? 'commonjs'}"
}`;