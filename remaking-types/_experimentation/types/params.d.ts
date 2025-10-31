// Based off of @types/express
type RouteParams<Path extends string> = string extends Path ? Record<string, string> : Path extends `${string}:${infer Param}/${infer Rest}` ? { [K in Param | keyof RouteParams<`/${Rest}`>]: string } : Path extends `${string}:${infer Param}` ? { [K in Param]: string } : {}

type Handler<Path extends string> = (req: {
    params: RouteParams<Path>
}) => void;

export function get<Path extends string>(path: Path, handler: Handler<Path>): void;