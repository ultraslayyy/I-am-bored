import { Table } from '../dist/table';
import { Enum } from '../dist/enum';
import { uuid, serial } from '../dist/column-builders';

export const themeOptions = new Enum('theme', ['light', 'dark', 'system']);

export const users = new Table('users', {
    id: uuid('id', { primaryKey: true }).defaultRandom()
});