export const regexList: RegExp[] = [];

export type DateReturnTypes = {
    'ISOString': 'toISOString';
    'UTCString': 'toUTCString';
    'DateString': 'toDateString';
    'LocaleDateString': 'toLocaleDateString';
    'LocaleString': 'toLocaleString';
    'LocaleTimeString': 'toLocaleTimeString';
    'TimeString': 'toTimeString';
    'Date': 'Date';
}

export type DateMethodName = DateReturnTypes[keyof DateReturnTypes];