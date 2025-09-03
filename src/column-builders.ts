import { Column } from './column.js';
import { DataType } from './types.js';

export function text(opts = {}) {
    return Column(DataType.TEXT, { ...opts });
}

export function serial(opts = {}) {
    return Column(DataType.SERIAL, { ...opts });
}