export type SQLQuery = {
    text: string;
    values: unknown[];
}
export type SQL = {
    (strings: TemplateStringsArray, ...values: unknown[]): SQLQuery;
}

export const sql: unique symbol;
export const and: string;
export const eq: string;
export const relations: string;