import { Add, Subtract } from './math';

type Multiply<A extends number, B extends number, R extends number = 0> = B extends 0 ? R : Multiply<A, Subtract<B, 1>, Add<R, A>>;

export type IsPythagorean<A extends number, B extends number, C extends number> =
    Multiply<A, A> extends infer ASquared
        ? ASquared extends number
            ? Multiply<B, B> extends infer BSquared
                ? BSquared extends number
                    ? Add<ASquared, BSquared> extends Multiply<C, C>
                        ? true
                        : false
                    : false
                : false
            : false
        : false;