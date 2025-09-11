import { drizzle } from './types/postgres-js';
import * as schema from './schema';
import { eq } from './types';

const db = drizzle({}, { schema });

const users = await db.query.users.findMany({
    
});