import { drizzle } from './types/postgres-js';
import * as schema from './schema';
import { and, sql } from './types';

const db = drizzle({}, { schema });