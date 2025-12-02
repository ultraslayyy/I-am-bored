import { ParseJSONString } from './types/parseJSON';

type userData = ParseJSONString<'{"name":"Alice","age":30,"isAdmin":true}'>;