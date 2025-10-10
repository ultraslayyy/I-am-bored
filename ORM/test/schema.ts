import { Table } from '../dist/table';
import { uuid } from '../dist/column-builders';

const users = new Table('users', {
    id: uuid('id').defaultRandom()
});