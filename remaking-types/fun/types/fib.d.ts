import { Add } from './math';

export type Fib<N extends number, Current extends number = 1, Prev extends number = 0, Curr extends number = 1> =
    N extends 0 ? 0 :
    N extends Current
        ? Curr
        : Add<Prev, Curr> extends infer Next
            ? Next extends number
                ? Fib<N, Add<Current, 1>, Curr, Next>
                : never
            : never;