type Base = 'A' | 'T' | 'C' | 'G';
export type DNAArray = readonly Base[];

type ValidDNA<S extends string> = S extends `${infer F}${infer R}` ? F extends Base ? ValidDNA<R> : never : S;
type DNAString = string & { __brand: 'DNA' }

type Split<T extends readonly any[], N extends number, L extends any[] = []> = L['length'] extends N ? [L, T] : T extends [infer F, ...infer R] ? Split<R, N, [...L, F]> : [L, T];
type Concat<A extends any[], B extends any[]> = [...A, ...B];

type MultiCrossover<A extends DNAArray, B extends DNAArray, Points extends number[], Flip extends boolean = false> = Points extends [infer P extends number, ...infer Rest extends number[]] ? Split<A, P> extends [infer A1 extends any[], infer A2 extends any[]] ? Split<B, P> extends [infer B1 extends any[], infer B2 extends any[]] ? Flip extends false ? Concat<A1, MultiCrossover<A2, B2, Rest, true>> : Concat<B1, MultiCrossover<A2, B2, Rest, false>> : [] : [] : Flip extends false ? A : B;
type Recombine<A extends DNAArray, B extends DNAArray, Points extends number[]> = [MultiCrossover<A, B, Points, false>, MultiCrossover<A, B, Points, true>];

type ChromosomeID = '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9' | '10' | '11' | '12' | '13' | '14' | '15' | '16' | '17' | '18' | '19' | '20' | '21' | '22' | 'X' | 'Y';
interface Chromosome<ID extends ChromosomeID, DNA extends DNAArray> {
    id: ID,
    dna: DNA
}
type ChromosomePair<ID extends ChromosomeID, A extends DNAArray, B extends DNAArray> = readonly [Chromosome<ID, A>, Chromosome<ID, B>];

type Chromosome21Pair = readonly [Chromosome<'21', DNAArray>, Chromosome<'21', DNAArray>] | readonly [Chromosome<'21', DNAArray>, Chromosome<'21', DNAArray>, Chromosome<'21', DNAArray>];

type Genome = {
    [K in Exclude<ChromosomeID, '21'>]: ChromosomePair<K, DNAArray, DNAArray>;
} & {
    '21': Chromosome21Pair;
}

type Meiosis<ID extends ChromosomeID, A extends DNAArray, B extends DNAArray, Points extends number[]> = Recombine<A, B, Points> extends [infer C1 extends DNAArray, infer C2 extends DNAArray] ? Chromosome<ID, C1> | Chromosome<ID, C2> : never;
type Gamete<G extends Genome> = {
    [K in keyof G]: G[K] extends ChromosomePair<infer ID, infer A extends DNAArray, infer B extends DNAArray> ? Meiosis<ID, A, B, [1, 2]> : never;
}

type Fertilise<A extends Record<any, any>, B extends Record<any, any>> = {
    [K in keyof A]: K extends keyof B ? readonly [A[K], B[K]] : never;
}

type MutateBase<B extends Base> =
    B extends 'A' ? 'T' :
    B extends 'T' ? 'A' :
    B extends 'C' ? 'G' :
    B extends 'G' ? 'C' : never;
type Mutate<T extends DNAArray> = {
    [K in keyof T]: MutateBase<T[K]>;
}
type PointMutate<T extends DNAArray, Index extends number, I extends any[] = [], R extends any[] = []> = T extends [infer F extends Base, ...infer Rest extends Base[]] ? I['length'] extends Index ? [...R, MutateBase<F>, ...Rest] : PointMutate<Rest, Index, [...I, any], [...R, F]> : R;

type Dominance = 'dominant' | 'recessive';

interface Allele<Trait extends string> {
    trait: Trait;
    dominance: Dominance;
}
type AllelePair<T extends string> =readonly [Allele<T>, Allele<T>];
type Express<A extends Allele<any>, B extends Allele<any>> = A['dominance'] extends 'dominant' ? A['trait'] : B['trait'];

type Take<T extends DNAArray, N extends number, R extends Base[] = []> = R['length'] extends N ? R : T extends readonly [infer F extends Base, ...infer Rest extends Base[]] ? Take<Rest, N, [...R, F]> : R;

type HasDownSyndrome<G extends Genome> = G['21']['2'] extends Chromosome<'21', DNAArray> ? true : false;


type EpigeneticEffect =
    | { type: 'silence' }
    | { type: 'enhance'; factor: number }
    | { type: 'mutate'; map: Partial<Record<Base, Base>> }

type EpigeneticMark<ID extends ChromosomeID = ChromosomeID> = {
    chromosome: ID;
    start: number;
    end: number;
    effect: EpigeneticEffect;
}


type EyeColour = 'blue' | 'green' | 'brown';
type EyePair = readonly [EyeColour, EyeColour];

type BuildTuple<N extends number, R extends any[] = []> = R['length'] extends N ? R : BuildTuple<N, [...R, any]>;
type Add<A extends number, B extends number> = [...BuildTuple<A>, ...BuildTuple<B>]['length'];

type Gene15<T extends DNAArray> = Take<T, 5>;
type Gene19<T extends DNAArray> = Take<T, 3>;
type Gene5<T extends DNAArray>  = Take<T, 2>;

type Pigment15<T extends DNAArray> =
    Gene15<T> extends ['A', 'A', 'A', 'A', 'A'] ? 5 :
    Gene15<T> extends ['A', 'T', 'A', 'T', 'A'] ? 3 :
    Gene15<T> extends ['T', 'T', 'T', 'T', 'T'] ? 0 :
    2;

type Pigment19<T extends DNAArray> =
    Gene19<T> extends ['A', 'A', 'A'] ? 2 :
    Gene19<T> extends ['T', 'T', 'T'] ? 0 :
    1;

type Pigment5<T extends DNAArray> =
    Gene5<T> extends ['A', 'A'] ? 1 :
    Gene5<T> extends ['T', 'T'] ? 0 :
    1;

type TotalPigment<C15 extends number, C19 extends number, C5 extends number> = Add<Add<C15, C19> extends infer T extends number ? T : never, C5> extends infer R extends number ? R : never;

type PigmentToColour<N extends number> = N extends 0 | 1 | 2 ? 'blue' : N extends 3 | 4 | 5 ? 'green' : 'brown';

type EyeFromChromosomes<C15 extends DNAArray, C19 extends DNAArray, C5 extends DNAArray> = PigmentToColour<TotalPigment<Pigment15<C15>, Pigment19<C19>, Pigment5<C5>>>;

type EyePairFromGenome<G extends Genome> = G['15'] extends ChromosomePair<any, infer A15 extends DNAArray, infer B15 extends DNAArray> ? G['19'] extends ChromosomePair<any, infer A19 extends DNAArray, infer B19 extends DNAArray> ? G['5'] extends ChromosomePair<any, infer A5 extends DNAArray, infer B5 extends DNAArray> ? [EyeFromChromosomes<A15, A19, A5>, EyeFromChromosomes<B15, B19, B5>] : ['blue', 'blue'] : ['blue', 'blue'] : ['blue', 'blue'];

type IsHeterochromia<E extends EyePair> = E[0] extends E[1] ? false : true;

type EyeFinal<G extends Genome, Manual extends EyePair | undefined = undefined> = EyePairFromGenome<G> extends infer E extends EyePair ? E extends EyePair ? E : Manual extends EyePair ? Manual : EyePair : Manual extends EyePair ? Manual : EyePair;


type HairColour = 'blonde' | 'brown' | 'black' | 'red';

type HairGene16<T extends DNAArray> = Take<T, 2>;
type HairGene15<T extends DNAArray> = Take<T, 4>;
type HairGene5<T extends DNAArray> = Take<T, 2>;

type IsRedGene<T extends DNAArray> = HairGene16<T> extends ['T', 'T'] ? true : false;

type HairPigment15<T extends DNAArray> =
    HairGene15<T> extends ['A', 'A', 'A', 'A'] ? 3 :
    HairGene15<T> extends ['T', 'T', 'T', 'T'] ? 0 :
    2;
type HairPigment5<T extends DNAArray> =
    HairGene5<T> extends ['A', 'A'] ? 1 :
    HairGene5<T> extends ['T', 'T'] ? 0 :
    1;

type HairTotalPigment<A extends number, B extends number> = Add<A, B> extends infer R extends number ? R : never;
type PigmentToHair<N extends number> = N extends 0 | 1 ? 'blonde' : N extends 2 | 3 ? 'brown' : 'black';

type HairFromChromosomes<C16 extends DNAArray, C15 extends DNAArray, C5 extends DNAArray> = IsRedGene<C16> extends true ? 'red' : PigmentToHair<HairTotalPigment<HairPigment15<C15>, HairPigment5<C5>>>;

type CombineHair<A extends HairColour, B extends HairColour> =
    A extends 'red' ? 'red' :
    B extends 'red' ? 'red' :
    A extends 'black' ? 'black' :
    B extends 'black' ? 'black' :
    A extends 'brown' ? 'brown' :
    B extends 'brown' ? 'brown' :
    'blonde';

type HairFromGenome<G extends Genome> = G['16'] extends ChromosomePair<any, infer A16 extends DNAArray, infer B16 extends DNAArray> ? G['15'] extends ChromosomePair<any, infer A15 extends DNAArray, infer B15 extends DNAArray> ? G['5'] extends ChromosomePair<any, infer A5 extends DNAArray, infer B5 extends DNAArray> ? CombineHair<HairFromChromosomes<A16, A15, A5>, HairFromChromosomes<B16, B15, B5>> : 'brown' : 'brown' : 'brown';


type Human<G extends Genome, ManualEyes extends EyePair | undefined = undefined> = {
    genome: G;

    traits: {
        eyes: EyeFinal<G, ManualEyes>;
        heterochromia: IsHeterochromia<EyeFinal<G, ManualEyes>>;
        hair: HairFromGenome<G>;
    }
}