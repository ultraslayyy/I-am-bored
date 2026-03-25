type BuildTuple<L extends number, T extends unknown[] = []> = T['length'] extends L ? T : BuildTuple<L, [...T, unknown]>;

export type Add<A extends number, B extends number> = [...BuildTuple<A>, ...BuildTuple<B>]['length'] extends infer Sum ? Sum extends number ? Sum : never : never;

export type Subtract<A extends number, B extends number> = BuildTuple<A> extends [...BuildTuple<B>, ...infer Rest] ? Rest['length'] : never;

type GreaterThan<A extends number, B extends number, T extends unknown[] = BuildTuple<A>> = T extends [...BuildTuple<B>, ...infer Rest] ? Rest['length'] extends 0 ? false : true : false;