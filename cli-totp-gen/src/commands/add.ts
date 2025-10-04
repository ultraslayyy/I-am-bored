import Command, { Param } from '../cmd';
import { parseOtpauthUri } from '../utils/otp';
import { addToken } from '../utils/storage';

export default class add extends Command {
    static commandName: string = 'add';
    static description = '';
    static params: Param[] = [
        { name: 'uri', type: 'string', description: 'otpauth:// URI' },
        { name: 'type', type: 'string', description: 'OTP type: totp | hotp (default: totp)', default: 'totp' },
        { name: 'label', type: 'string', description: 'Label (name) for otp key (Required if no uri)' },
        { name: 'secret', type: 'string', description: 'OTP secret (Required if no uri)' },
        { name: 'issuer', type: 'string', description: 'Issuer name' },
        { name: 'digits', type: 'number', description: 'Number of digits (default: 6)', default: '6' },
        { name: 'step', type: 'number', description: 'Time period step in seconds (TOTP only, default: 30)', default: '30' },
        { name: 'counter', type: 'number', description: 'HOTP counter (HOTP only, default: 0)', default: '0' },
        { name: 'algorithm', type: 'string', description: 'Algorithm: sha1, sha256, or sha512 (default: sha1)', default: 'sha1' },
        { name: 'force', type: 'boolean', description: 'Allow overiding existing labels, if they exist' },
        { name: 'encrypt', type: 'boolean', description: 'Encrypt secrets on disk' },
    ];
    static usage: string[] = ['votp add --uri <otpauth URI> [flags]', 'votp add --label test --secret itsasecret [flags]'];

    async execute(cmdArgs: Record<string, any>, globals: Record<string, any>) {
        if (cmdArgs.uri) {
            const opts = parseOtpauthUri(cmdArgs.uri);
            if (opts.type === 'hotp') {
                addToken({
                    type: 'hotp',
                    secret: opts.secret,
                    label: opts.label,
                    digits: opts.digits,
                    issuer: opts.issuer,
                    algorithm: opts.algorithm,
                    counter: opts.counter
                }, cmdArgs.force);
            } else {
                addToken({
                    type: 'totp',
                    secret: opts.secret,
                    label: opts.label,
                    digits: opts.digits,
                    issuer: opts.issuer,
                    algorithm: opts.algorithm,
                    step: opts.step
                }, cmdArgs.force);
            }
        } else {
            if (cmdArgs.type === 'totp') {
                addToken({
                    type: 'totp',
                    secret: cmdArgs.secret,
                    label: cmdArgs.label,
                    digits: Number(cmdArgs.digits),
                    issuer: cmdArgs.issuer,
                    algorithm: cmdArgs.algorithm,
                    step: Number(cmdArgs.step)
                }, cmdArgs.force);
            } else {
                addToken({
                    type: 'hotp',
                    secret: cmdArgs.secret,
                    label: cmdArgs.label,
                    digits: Number(cmdArgs.digits),
                    issuer: cmdArgs.issuer,
                    algorithm: cmdArgs.algorithm,
                    counter: Number(cmdArgs.counter)
                }, cmdArgs.force);
            }
        }
    }
}