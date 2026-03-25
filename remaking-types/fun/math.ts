import { Add, Subtract, GreaterThan as Gt } from './types/math';

type add = Add<8, 2>;
type sub = Subtract<8, 2>;
type gt = Gt<5, 3>;