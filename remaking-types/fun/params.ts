// Based off of @types/express
import { get } from './types/params';

get('/user/:userId/books/:bookId', (req) => {
    req.params;
});