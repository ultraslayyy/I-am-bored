export type SQLQuery = {
    text: string;
    values: unknown[];
}
export type SQL<T> = {
    (strings: TemplateStringsArray, ...values: unknown[]): SQLQuery;
}

export function sql<T>(strings: TemplateStringsArray, ...params: any[]): SQL<T>;
export const and: string;
export const eq: string;
export const relations: string;