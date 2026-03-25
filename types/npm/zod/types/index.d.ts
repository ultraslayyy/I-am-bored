//#region Classes
export namespace z {
    export type ArrayCardinality = 'atleastone' | 'many';

    export class ZodArray<T extends ZodTypeAny, Cardinality extends ArrayCardinality = 'many'> extends ZodType<Cardinality extends 'atleastone' ? [ZodOutput<T>, ...ZodOutput<T>[]] : ZodOutput<T>[], any, Cardinality extends 'atleastone' ? [ZodInput<T>, ...ZodInput<T>[]] : ZodInput<T>[]> {
        readonly element: T;
        readonly arrayOptions?: Cardinality;
    }

    export class ZodBoolean extends ZodType<boolean> { }

    export class ZodEffects<T extends ZodTypeAny, Output = ZodOutput<T>, Input = ZodInput<T>> extends ZodType<Output, any, Input> {
        readonly inner: T;
    }

    export class ZodEnum<T extends readonly [string, ...string[]]> extends ZodType<T[number]> {
        readonly options: T;
    }

    export class ZodLiteral<T extends string | number | boolean> extends ZodType<T> {
        readonly value: T;
    }

    export class ZodNumber extends ZodType<number> { }

    type InferObjectOutput<Shape extends Record<string, ZodTypeAny>> = {
        [K in keyof Shape]: ZodOutput<Shape[K]>
    };

    type InferObjectInput<Shape extends Record<string, z.ZodTypeAny>> = {
        [K in keyof Shape]: ZodInput<Shape[K]>;
    }

    export class ZodObject<Shape extends Record<string, ZodTypeAny>, Mode extends 'strip' | 'passthrough' | 'strict' = 'strip', Def = any, Output extends Partial<{ [K in keyof Shape]: ZodOutput<Shape[K]> }> = InferObjectOutput<Shape>, Input extends Partial<{ [K in keyof Shape]: ZodInput<Shape[K]> }> = InferObjectInput<Shape>> extends ZodType<Mode extends 'strip' ? InferObjectOutput<Shape> : Output, Def, Input> {
        readonly shape: Shape;
        readonly mode: Mode;
    }

    export class ZodOptional<T extends ZodTypeAny> extends ZodType<ZodOutput<T> | undefined, any, ZodInput<T> | undefined> {
        readonly inner: T;
    }

    export class ZodString extends ZodType<string> { }

    export type RawCreateParams = {
        errorMap?: undefined; // TODO
        invalid_type_error?: string | undefined;
        required_error?: string | undefined;
        message?: string | undefined;
        description?: string | undefined;
    } | undefined;

    export class ZodTuple<T extends readonly ZodTypeAny[]> extends ZodType<{ [K in keyof T]: ZodOutput<T[K]> }, any, { [K in keyof T]: ZodInput<T[K]> }> {
        readonly items: T;
    }

    export abstract class ZodType<Output = any, Def = any, Input = Output> {
        readonly _type: Output;
        readonly _output: Output;
        readonly _input: Input;
        readonly _def: Def;
        get description(): string | undefined;
        describe(description: string): this;
        isOptional(): boolean;
        isNullable(): boolean;

        parse: (data: Input) => Output;
    }

    export class ZodTypeAny extends ZodType<any> { }

    export class ZodUnion<T extends readonly [ZodTypeAny, ...ZodTypeAny[]]> extends ZodType<ZodOutput<T[number]>, any, ZodInput<T[number]>> {
        readonly options: T;
    }

    export type ZodInput<T> = T extends ZodType<any, any, infer I> ? I : never;
    export type input<T extends ZodType> = T['_input'];
    export type ZodOutput<T> = T extends ZodType<infer O> ? O : never;
}

//#endregion



//#region Output

export const zod: {
    string(): z.ZodString;
    number(): z.ZodNumber;
    literal<T extends string | number | boolean>(value: T, params?: z.RawCreateParams): z.ZodLiteral<T>;
    tuple<T extends readonly [z.ZodTypeAny, ...z.ZodTypeAny[]] | []>(schemas: T, params?: z.RawCreateParams): z.ZodTuple<T>;
    union<T extends readonly [z.ZodTypeAny, ...z.ZodTypeAny[]]>(types: T, params?: z.RawCreateParams): z.ZodUnion<T>;
    object<T extends Record<string, z.ZodTypeAny>>(shape: T): z.ZodObject<T>;
    optional<T extends z.ZodTypeAny>(inner: T): z.ZodOptional<T>;
    array<T extends z.ZodTypeAny, Cardinality extends z.ArrayCardinality = 'many'>(inner: T, cardinality: Cardinality): z.ZodArray<T, Cardinality>;
    enum<T extends readonly [string, ...string[]]>(vals: T): z.ZodEnum<T>;
    effects<T extends z.ZodTypeAny>(inner: T): z.ZodEffects<T>;
}

export default z;
export as namespace z;

//#endregion