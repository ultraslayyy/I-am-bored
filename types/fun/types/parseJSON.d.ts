type JSONPrimitive = string | number | boolean | null;

export type ParseJSONString<S extends string> =
    S extends `{"${infer K}":${infer V},${infer Rest}`
        ? { [P in K]: ParseJSONValue<V> } & ParseJSONString<`{${Rest}`>
        : S extends `{"${infer K}":${infer V}}`
            ? { [P in K]: ParseJSONValue<V> }
            : {}

export type ParseJSONValue<V extends string> =
    V extends "true" ? true :
    V extends "false" ? false :
    V extends "null" ? null :
    V extends `${infer N extends number}` ? N :
    V extends `"${infer S}"` ? S :
    never;