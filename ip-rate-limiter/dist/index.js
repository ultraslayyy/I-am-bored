const defaultOptions = {
    windowMs: 15 * 60 * 1000,
    max: 100,
    message: 'Too many requests, please try again later.',
    statusCode: 429,
    keyGenerator: (req) => req.ip,
    headers: true
};

export function rateLimiter(userOptions = {}) {
    const options = { ...defaultOptions, ...userOptions };
    const hits = new Map();

    setInterval(() => {
        const now = Date.now();
        for (const [key, entry] of hits.entries()) {
            if (entry.expires < now) {
                hits.delete(key);
            }
        }
    }, 60 * 1000);

    return function (req, res, next) {
        const key = options.keyGenerator(req);
        const now = Date.now();
        const resetTime = now + options.windowMs;

        if (!hits.has(key)) {
            hits.set(key, {
                count: 1,
                expires: resetTime
            });
        } else {
            const entry = hits.get(key);
            if (entry.expires > now) {
                entry.count++;
                if (entry.count > options.max) {
                    if (options.headers) {
                        res.setHeader('Retry-After', Math.ceil((entry.expires - now) / 1000));
                    }
                    return res.status(options.statusCode).send(typeof options.message === 'function' ? options.message(req) : options.message); 
                }
            } else {
                hits.set(key, {
                    count: 1,
                    expires: resetTime
                });
            }
        }

        if (options.headers) {
            const entry = hits.get(key);
            res.setHeader('X-RateLimit-Limit', options.max);
            res.setHeader('X-RateLimit-Remaining', Math.max(options.max - entry.count, 0));
            res.setHeader('X-RateLimit-Reset', Math.floor(entry.expires / 1000));
        }

        next();
    }
}

export default rateLimiter;