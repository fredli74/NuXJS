var d;



console = { log:print };

// console.log( (d = new Date(1603585029123 + -3*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )
// console.log( (d = new Date(1603585029123 + -2*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )
// console.log( (d = new Date(1603585029123 + -1*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )
// console.log( (d = new Date(1603585029123 + 0*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )
// console.log( (d = new Date(1603585029123 + 1*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )
// console.log( (d = new Date(1603585029123 + 2*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )
// console.log( (d = new Date(1603585029123 + 3*60*60*1000)).valueOf() + " = " + String( d ) + " / " + d.toUTCString() + " / " + d.toISOString() + " ("+$support.localTimeDifference(d.valueOf())+")" )


// console.log("2020-10-25 00:17:09.123 = " + (new Date(2020,9,25,0,17,9,123)).valueOf() + " == 1603577829123 / " + (new Date(2020,9,25,0,17,9,123)).toString());
// console.log("2020-10-25 01:17:09.123 = " + (new Date(2020,9,25,1,17,9,123)).valueOf() + " == 1603581429123 / " + (new Date(2020,9,25,1,17,9,123)).toString());
// console.log("2020-10-25 02:17:09.123 = " + (new Date(2020,9,25,2,17,9,123)).valueOf() + " == 1603585029123 / " + (new Date(2020,9,25,2,17,9,123)).toString());
// console.log("2020-10-25 03:17:09.123 = " + (new Date(2020,9,25,3,17,9,123)).valueOf() + " == 1603592229123 / " + (new Date(2020,9,25,3,17,9,123)).toString());

// console.log("2020-03-29 00:17:09.123 = " + (new Date(2020,2,29,0,17,9,123)).valueOf() + " == 1585437429123 / " + (new Date(2020,2,29,0,17,9,123)).toString());
// console.log("2020-03-29 01:17:09.123 = " + (new Date(2020,2,29,1,17,9,123)).valueOf() + " == 1585441029123 / " + (new Date(2020,2,29,1,17,9,123)).toString());
// console.log("2020-03-29 02:17:09.123 = " + (new Date(2020,2,29,2,17,9,123)).valueOf() + " == 1585444629123 / " + (new Date(2020,2,29,2,17,9,123)).toString());
// console.log("2020-03-29 03:17:09.123 = " + (new Date(2020,2,29,3,17,9,123)).valueOf() + " == 1585444629123 / " + (new Date(2020,2,29,3,17,9,123)).toString());

console.log( (new Date(2020,9,25,1,17,9,123)).toString() + " = " + (new Date(2020,9,25,1,17,9,123)).valueOf() );
console.log( (new Date(2020,9,25,2,17,9,123)).toString() + " = " + (new Date(2020,9,25,2,17,9,123)).valueOf() );
console.log( (new Date(2020,9,25,3,17,9,123)).toString() + " = " + (new Date(2020,9,25,3,17,9,123)).valueOf() );
console.log("");

console.log( (new Date()).toString() );

console.log( $support.localTimeDifference( -51506840571000 ) );
console.log( (new Date(-51506840571000)).toISOString() ); 
console.log( (new Date(-51506840571000)).toString() ); 

console.log((new Date(377,2,27,1,44,18)).valueOf());
console.log((new Date(377,2,27,1,44,18)).toISOString());
console.log((new Date(-50262851742000 + 60*60*1000)).toISOString());

console.log(new Date(1999,9,31,2,13,41).valueOf());

console.log(new Date(-1999,9,20,2,13,41).toISOString());

console.log(new Date(1974,6,14,1,15,16,17).toString());
console.log(new Date(1974,6,14,1,15,16,17).toUTCString());
console.log(new Date(1974,6,14,1,15,16,17).toISOString());
console.log(new Date(12345,6,14,1,15,16,17).toString());
console.log(new Date(123456,6,14,1,15,16,17).toUTCString());
