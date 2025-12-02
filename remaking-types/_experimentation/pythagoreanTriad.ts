import { IsPythagorean } from './types/pythagoreanTriad';

type IsTriad = IsPythagorean<3, 4, 5>;
type IsNotTriad = IsPythagorean<2, 3, 4>;