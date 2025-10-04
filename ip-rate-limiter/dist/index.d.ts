import { Request, Response, NextFunction } from 'express';

export interface RateLimitOptions {
    windowMs?: number;
    max?: number;
    message?: string | ((req: Request) => string);
    statusCode?: number;
    keyGenerator?: (req: Request) => string;
    headers?: boolean;
}

export type RateLimitMiddleware = (req: Request, res: Response, next: NextFunction) => void;

export declare function rateLimiter(options?: RateLimitOptions): RateLimitMiddleware;
export default rateLimiter;