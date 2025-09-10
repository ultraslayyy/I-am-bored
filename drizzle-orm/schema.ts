import { customType, pgEnum, pgTable } from './types/pg-core';
import { eq } from './types';

export const themeOptions = pgEnum('theme', ['light', 'dark', 'system']);
export const mfaMethods = pgEnum('mfa_methods', ['otp', 'webauthn']);
export const rcptTypes = pgEnum('rcpt_types', [
    'to',
    'cc',
    'bcc'
]);

const tsvector = customType<{ data: string; notNull: false, default: false; }>({
    dataType() {
        return 'tsvector';
    }
});