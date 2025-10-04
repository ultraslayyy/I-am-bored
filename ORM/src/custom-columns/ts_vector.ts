import { ColumnType, type ColumnOptions } from '../column-types.js';

export type TsVectorOptions = {
    
}

export class TsVectorColumn extends ColumnType {
    constructor(name: string, tsvectorOptions: TsVectorOptions, options?: ColumnOptions) {
        super(name, options);
    }

    toSQL(name: string): string {
        return '';
    }
}