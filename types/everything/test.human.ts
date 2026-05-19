import { ChromosomeID, DNAArray, Chromosome, Genome, Human, HasDownSyndrome } from './human';

const chr = <ID extends ChromosomeID, const T extends DNAArray>(id: ID, dna: T): Chromosome<ID, T> => ({ id, dna });

const genome = {
    '5': [
        chr('5', ['A', 'A']),
        chr('5', ['T', 'T'])
    ],
    '15': [
        chr('15', ['A', 'A', 'A', 'A', 'A']), // brown eye
        chr('15', ['T', 'T', 'T', 'T', 'T'])  // blue  eye
    ],
    '16': [
        chr('16', ['A', 'T']),
        chr('16', ['A', 'T'])
    ],
    '19': [
        chr('19', ['A', 'A', 'A']),
        chr('19', ['T', 'T', 'T'])
    ],

    '1':  [chr('1', []), chr('1', [])],
    '2':  [chr('2', []), chr('2', [])],
    '3':  [chr('3', []), chr('3', [])],
    '4':  [chr('4', []), chr('4', [])],
    '6':  [chr('6', []), chr('6', [])],
    '7':  [chr('7', []), chr('7', [])],
    '8':  [chr('8', []), chr('8', [])],
    '9':  [chr('9', []), chr('9', [])],
    '10': [chr('10', []), chr('10', [])],
    '11': [chr('11', []), chr('11', [])],
    '12': [chr('12', []), chr('12', [])],
    '13': [chr('13', []), chr('13', [])],
    '14': [chr('14', []), chr('14', [])],
    '17': [chr('17', []), chr('17', [])],
    '18': [chr('18', []), chr('18', [])],
    '20': [chr('20', []), chr('20', [])],
    '21': [chr('21', []), chr('21', [])],
    '22': [chr('22', []), chr('22', [])],
    'X':  [chr('X', []), chr('X', [])],
    'Y':  [chr('Y', []), chr('Y', [])]
} as const satisfies Genome;

type TestHuman = Human<typeof genome>;
type Eyes = TestHuman['traits']['eyes'];
type Hetero = TestHuman['traits']['heterochromia'];
type Down = HasDownSyndrome<typeof genome>;
type Hair = TestHuman['traits']['hair'];