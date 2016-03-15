var STRIDE = (8 + 2);

var BOARD_SIZE = (8 + 4) * STRIDE;

function fillArray(a, value) {
	for (var i = 0; i < a.length; ++i) a[i] = value;
	return a;
}

print(typeof BOARD_SIZE);

function create1DArray(dim, initValue) {
	return fillArray(new Array(dim), initValue);
};

print(typeof BOARD_SIZE);

var board = create1DArray(BOARD_SIZE, 0);

