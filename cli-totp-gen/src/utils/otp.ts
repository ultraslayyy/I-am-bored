import { totp as totpLib, hotp as hotpLib } from 'otplib';
import { HashAlgorithms } from 'otplib/core';
import { URL } from 'node:url';

type Algorithm = 'sha1' | 'sha256' | 'sha512';
type OtpType = 'totp' | 'hotp';

interface BaseOtpOptions {
    secret: string;
    label: string;
    digits?: number;
    issuer?: string;
    algorithm?: HashAlgorithms;
}

interface TotpOptions extends BaseOtpOptions {
    type: 'totp';
    step?: number;
}

export interface HotpOptions extends BaseOtpOptions {
    type: 'hotp';
    counter: number;
}

export type OtpOptions = TotpOptions | HotpOptions;

const algorMap: Record<Algorithm, HashAlgorithms> = {
    sha1: HashAlgorithms.SHA1,
    sha256: HashAlgorithms.SHA256,
    sha512: HashAlgorithms.SHA512
} as const;

export function generateOtp(options: OtpOptions): string {
    const { secret, digits = 6, algorithm: algorithmRaw = 'sha1' } = options;

    const algorithm = algorMap[algorithmRaw];

    if (options.type === 'totp') {
        totpLib.options = { digits, algorithm, step: options.step ?? 30 }
        return totpLib.generate(secret);
    } else {
        hotpLib.options = { digits, algorithm }
        return hotpLib.generate(secret, options.counter);
    }
}

export function validateOtp(options: OtpOptions, token: string) {
    const { secret, digits = 6, algorithm: algorithmRaw = 'sha1' } = options;

    const algorithm = algorMap[algorithmRaw];

    if (options.type === 'totp') {
        totpLib.options = { digits, algorithm, step: options.step ?? 30 }
        return totpLib.check(token, secret);
    } else {
        hotpLib.options = { digits, algorithm }
        return hotpLib.check(token, secret, options.counter);
    }
}

export type ParsedOtpEntry =
    | {
          type: 'totp';
          label: string;
          secret: string;
          issuer?: string;
          digits?: number;
          step?: number;
          algorithm?: Algorithm;
      }
    | {
          type: 'hotp';
          label: string;
          secret: string;
          counter: number;
          issuer?: string;
          digits?: number;
          algorithm?: Algorithm;
      }

export function parseOtpauthUri(uri: string): OtpOptions {
    if (!uri.startsWith('otpauth://')) {
        throw new Error('Invalid otpauth URI');
    }

    const parsed = new URL(uri);

    const otpType = parsed.hostname as OtpType;
    const labelRaw = decodeURIComponent(parsed.pathname.slice(1));
    const [issuerFromLabel, label] = labelRaw.includes(':')
        ? labelRaw.split(/:(.+)/)
        : [undefined, labelRaw];

    const params = parsed.searchParams;
    const secret = params.get('secret');
    const issuer = params.get('issuer') || issuerFromLabel;
    const digits = params.has('digits') ? parseInt(params.get('digits')!) : undefined;

    const algorithmRaw = params.get('algorithm')?.toLowerCase();
    const algorithm = algorithmRaw ? algorMap[algorithmRaw as Algorithm] : undefined;

    if (!secret) throw new Error('Missing secret in otpauth URI');

    if (otpType === 'totp') {
        const step = params.has('period') ? parseInt(params.get('period')!) : undefined;
        return {
            type: 'totp',
            secret,
            label,
            issuer,
            digits,
            step,
            algorithm
        }
    } else if (otpType === 'hotp') {
        const counter = params.has('counter') ? parseInt(params.get('counter')!) : 6;
        return {
            type: 'hotp',
            secret,
            label,
            issuer,
            digits,
            counter,
            algorithm
        }
    }

    throw new Error(`Unsupported OTP type: ${otpType}`);
}