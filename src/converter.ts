import { regexList, DateMethodName } from './constants';

export default function convertDate(date: string | number, returnValue: DateMethodName) {
    let newDate: Date;
    if (typeof date === 'number') {
        const length = Math.floor(date).toString().replace('-', '').length;

        if (length === 10) {
            newDate = new Date(date * 1000);
        } else if (length === 13) {
            newDate = new Date(date);
        } else return 'Invalid number';

        if (isNaN(newDate.getTime())) return 'Invalid date';
    } else {
        const d = new Date(date);
        if (!isNaN(d.getTime())) return d.toISOString();
        newDate = new Date();
    }

    if (returnValue === 'Date') {
        return newDate;
    } else {
        return newDate[returnValue]();
    }
}