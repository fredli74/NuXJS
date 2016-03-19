glob4 = 'abcd';
print(delete glob4);
print(typeof glob4);
glob4 = 'efgh';

var glob4;
print(delete glob4);
print(typeof glob4);

var glob1 = 'glob';
print('glob1' in this);

if (false) { var glob2 = 'glob'; }
print('glob2' in this);

this.glob1 = 'eh';
print(delete glob1);
print(typeof glob1);

this.glob3 = 'eh';
print(delete glob3);
print(typeof glob3);

/*Object.defineProperty(this, 'glob4', { value: 1234, configurable: true });
print(delete glob4);
print(typeof glob4);*/
