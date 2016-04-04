Array = function() { };
seed = 2463534242;
myrand = function() {
	var y = seed;
	y ^= y << 13;
	y ^= y >>> 17;
	y ^=  (y << 5);
	return (seed = y);
};
var memory = new Array();
for (var i = 0; i < 50; ++i) {
	for (var j = 0; j < 20000; ++j) memory[j] = myrand() >>> 0;
}
var f;
var z = -12345;
// startClock = clock();
for (var i = 0; i < 200; ++i) {
	f = 0;
	for (var j = 1; j < 20000; ++j) if (memory[j] < memory[f]) f = j
}
for (var j = 0; j < 20000; j += 2000) print(memory[j]);
print(f);
print(memory[f]);
f=function(n) { var i = 0; while (i < n) { f[i] = i * 1234; ++i } }
f(1000000)
for (var i = 0; i < 1000000; i += 100000) print(i + ': ' + f[i]);
f=function(n) { var i = 0; while (i < n) { f[i] = i * 1234; ++i } };
for (var a = 0; a < 100000; ++a) f(100);
f=function(n) { var i = 0; while (i < n) { f[i] = "" + i * 1234; ++i } };
for (var a = 0; a < 2; ++a) f(1000000);
mandelbrot = function() {
	var I, R, s, b, n, out = "";
	eval('for(var i=-1, s=""; i<1; i+=0.06, out += s + "\\n", s="") for (var r=-2;(I=i,(R=r)<1);r+=0.03,s+=String.fromCharCode(n+31)) for(n=0;(b=I*I,26>n++&&R*R+b<4);I=2*R*I+i,R=R*R-b+r);');
	return out;
}
for (var i = 0; i < 100; ++i) s = mandelbrot();
print(s)
y = 0;
for (var j = 0; j < 10000; ++j)
	for (var i = 0; i < 100; ++i)
		eval("y = y + i * 3");
print(y)
// Not finished, was going to do heavy graphs to test gc
var o = function() { }
var a = new o;
for (var i = 0; i < 10000; ++i) {
	a[i] = new o;
	for (var j = 0; j < 100; ++j) {
		a[i][j] = new o;
	}
}
mandelbrot = function() {
	var I, R, s, b, n, out = "";
	for(var i=-1, s=''; i<1; i+=0.06, out += s + "\n", s='') {
		for (var r=-2;(I=i,(R=r)<1);r+=0.03,s+=String.fromCharCode(n+31)) {
			for(n=0;(b=I*I,26>n++&&R*R+b<4);I=2*R*I+i,R=R*R-b+r);
		}
	}
	return out;
}
for (var i = 0; i < 200; ++i) s = mandelbrot();
print(s)
var LOG2 = 0.69314718055994530942
var LOG2R = 1.44269504088896340736
var LOG10R = 0.43429448190325182765
var E = 2.71828182845904523536
var HALF_PI = 1.57079632679489661923
var PI = 3.14159265358979323846
var TWICE_PI = PI * 2.0
var COS_EPSILON = 1.0e-6

function abs(x) {
    return (x < 0 ? -x : x);
}

/*
    Derived from:
    
              oo
             =====  n
         x   \     x
        e  =  >    --
             /     n!
             =====
             n = 0
    
    ... which is the same as:
        
        1   x   x x   x x x
        - + - + --- + ----- + ...
        1   1   1*2   1*2*3
    
    ... which is the same as:
    
        1   x   x x   x x x
        - + - + - - + - - - + ...
        1   1   1 2   1 2 3
        
    Negative x yields huge floating point precision problems so we negate x before and do 1.0 / y afterwards instead.
*/
function exp(x) {
    var m = abs(x);
    var a = 1.0;
    var n = 1.0;
    var y = 1.0;
    do {
        var t = y;
        a *= m / n;
        y += a;
        n += 1.0;
    } while (y != t);
    return (x < 0.0 ? 1.0 / y : y);
}

/*
    Derived from:
    
                      oo
                     =====            n
                     \         n + 1 x
        log(1 + x) =  >    (-1)      --
                     /                n
                     =====
                     n = 1

    Taylor for log is only reliable between -0.5 and 0.5, so we scale beforehand.
*/
function log(x)
{
    if (x <= 0.0 || x >= 1.0e38) {
        throw("Domain error");
    }
    var b = 0.0;
    while (x < 0.5) { x *= 2.0; b -= LOG2; }
    while (x > 1.5) { x *= 0.5; b += LOG2; }
    x = 1.0 - x;
    var a = -1.0;
    var n = 1.0;
    var y = 0.0;
    do {
        t = y;
        a *= x;
        y += a / n;
        n += 1.0;
    } while (y != t);
    return y + b;
}

print(exp(-0.1));
print(exp(22.1));
print(log(1.1));
print(log(429039122.1));

print(log(exp(1)));
print(log(exp(2)));
print(log(exp(3)));
print(log(exp(4)));

for (var f = 0; f < 10; f += 0.0001) {
    if (abs(log(exp(f)) - f) > f * 0.00000001) {
        throw("Precision error");
    }
}
var solver = null;
var nsFrameCounter = 0;

function runNavierStokes()
{
    solver.update();
    nsFrameCounter++;

    if(nsFrameCounter==15)
        checkResult(solver.getDens());
}

function checkResult(dens) {

    this.result = 0;
    for (var i=7000;i<7100;i++) {
        this.result+=~~((dens[i]*10));
    }

    if (this.result!=77) {
        throw(new Error("checksum failed"));
    }
}

function setupNavierStokes()
{
    solver = new FluidField(null);
    solver.setResolution(128, 128);
    solver.setIterations(20);
    solver.setDisplayFunction(function(){});
    solver.setUICallback(prepareFrame);
    solver.reset();
}

function tearDownNavierStokes()
{
    solver = null;
}

function addPoints(field) {
    var n = 64;
    for (var i = 1; i <= n; i++) {
        field.setVelocity(i, i, n, n);
        field.setDensity(i, i, 5);
        field.setVelocity(i, n - i, -n, -n);
        field.setDensity(i, n - i, 20);
        field.setVelocity(128 - i, n + i, -n, -n);
        field.setDensity(128 - i, n + i, 30);
    }
}

var framesTillAddingPoints = 0;
var framesBetweenAddingPoints = 5;

function prepareFrame(field)
{
    if (framesTillAddingPoints == 0) {
        addPoints(field);
        framesTillAddingPoints = framesBetweenAddingPoints;
        framesBetweenAddingPoints++;
    } else {
        framesTillAddingPoints--;
    }
}

// Code from Oliver Hunt (http://nerget.com/fluidSim/pressure.js) starts here.
function FluidField(canvas) {
    function addFields(x, s, dt)
    {
        for (var i=0; i<size ; i++ ) x[i] += dt*s[i];
    }

    function set_bnd(b, x)
    {
        if (b===1) {
            for (var i = 1; i <= width; i++) {
                x[i] =  x[i + rowSize];
                x[i + (height+1) *rowSize] = x[i + height * rowSize];
            }

            for (var j = 1; j <= height; j++) {
                x[j * rowSize] = -x[1 + j * rowSize];
                x[(width + 1) + j * rowSize] = -x[width + j * rowSize];
            }
        } else if (b === 2) {
            for (var i = 1; i <= width; i++) {
                x[i] = -x[i + rowSize];
                x[i + (height + 1) * rowSize] = -x[i + height * rowSize];
            }

            for (var j = 1; j <= height; j++) {
                x[j * rowSize] =  x[1 + j * rowSize];
                x[(width + 1) + j * rowSize] =  x[width + j * rowSize];
            }
        } else {
            for (var i = 1; i <= width; i++) {
                x[i] =  x[i + rowSize];
                x[i + (height + 1) * rowSize] = x[i + height * rowSize];
            }

            for (var j = 1; j <= height; j++) {
                x[j * rowSize] =  x[1 + j * rowSize];
                x[(width + 1) + j * rowSize] =  x[width + j * rowSize];
            }
        }
        var maxEdge = (height + 1) * rowSize;
        x[0]                 = 0.5 * (x[1] + x[rowSize]);
        x[maxEdge]           = 0.5 * (x[1 + maxEdge] + x[height * rowSize]);
        x[(width+1)]         = 0.5 * (x[width] + x[(width + 1) + rowSize]);
        x[(width+1)+maxEdge] = 0.5 * (x[width + maxEdge] + x[(width + 1) + height * rowSize]);
    }

    function lin_solve(b, x, x0, a, c)
    {
        if (a === 0 && c === 1) {
            for (var j=1 ; j<=height; j++) {
                var currentRow = j * rowSize;
                ++currentRow;
                for (var i = 0; i < width; i++) {
                    x[currentRow] = x0[currentRow];
                    ++currentRow;
                }
            }
            set_bnd(b, x);
        } else {
            var invC = 1 / c;
            for (var k=0 ; k<iterations; k++) {
                for (var j=1 ; j<=height; j++) {
                    var lastRow = (j - 1) * rowSize;
                    var currentRow = j * rowSize;
                    var nextRow = (j + 1) * rowSize;
                    var lastX = x[currentRow];
                    ++currentRow;
                    for (var i=1; i<=width; i++)
                        lastX = x[currentRow] = (x0[currentRow] + a*(lastX+x[++currentRow]+x[++lastRow]+x[++nextRow])) * invC;
                }
                set_bnd(b, x);
            }
        }
    }

    function diffuse(b, x, x0, dt)
    {
        var a = 0;
        lin_solve(b, x, x0, a, 1 + 4*a);
    }

    function lin_solve2(x, x0, y, y0, a, c)
    {
        if (a === 0 && c === 1) {
            for (var j=1 ; j <= height; j++) {
                var currentRow = j * rowSize;
                ++currentRow;
                for (var i = 0; i < width; i++) {
                    x[currentRow] = x0[currentRow];
                    y[currentRow] = y0[currentRow];
                    ++currentRow;
                }
            }
            set_bnd(1, x);
            set_bnd(2, y);
        } else {
            var invC = 1/c;
            for (var k=0 ; k<iterations; k++) {
                for (var j=1 ; j <= height; j++) {
                    var lastRow = (j - 1) * rowSize;
                    var currentRow = j * rowSize;
                    var nextRow = (j + 1) * rowSize;
                    var lastX = x[currentRow];
                    var lastY = y[currentRow];
                    ++currentRow;
                    for (var i = 1; i <= width; i++) {
                        lastX = x[currentRow] = (x0[currentRow] + a * (lastX + x[currentRow] + x[lastRow] + x[nextRow])) * invC;
                        lastY = y[currentRow] = (y0[currentRow] + a * (lastY + y[++currentRow] + y[++lastRow] + y[++nextRow])) * invC;
                    }
                }
                set_bnd(1, x);
                set_bnd(2, y);
            }
        }
    }

    function diffuse2(x, x0, y, y0, dt)
    {
        var a = 0;
        lin_solve2(x, x0, y, y0, a, 1 + 4 * a);
    }

    function advect(b, d, d0, u, v, dt)
    {
        var Wdt0 = dt * width;
        var Hdt0 = dt * height;
        var Wp5 = width + 0.5;
        var Hp5 = height + 0.5;
        for (var j = 1; j<= height; j++) {
            var pos = j * rowSize;
            for (var i = 1; i <= width; i++) {
                var x = i - Wdt0 * u[++pos];
                var y = j - Hdt0 * v[pos];
                if (x < 0.5)
                    x = 0.5;
                else if (x > Wp5)
                    x = Wp5;
                var i0 = x | 0;
                var i1 = i0 + 1;
                if (y < 0.5)
                    y = 0.5;
                else if (y > Hp5)
                    y = Hp5;
                var j0 = y | 0;
                var j1 = j0 + 1;
                var s1 = x - i0;
                var s0 = 1 - s1;
                var t1 = y - j0;
                var t0 = 1 - t1;
                var row1 = j0 * rowSize;
                var row2 = j1 * rowSize;
                d[pos] = s0 * (t0 * d0[i0 + row1] + t1 * d0[i0 + row2]) + s1 * (t0 * d0[i1 + row1] + t1 * d0[i1 + row2]);
            }
        }
        set_bnd(b, d);
    }

    function project(u, v, p, div)
    {
        var h = -0.5 / Math.sqrt(width * height);
        for (var j = 1 ; j <= height; j++ ) {
            var row = j * rowSize;
            var previousRow = (j - 1) * rowSize;
            var prevValue = row - 1;
            var currentRow = row;
            var nextValue = row + 1;
            var nextRow = (j + 1) * rowSize;
            for (var i = 1; i <= width; i++ ) {
                div[++currentRow] = h * (u[++nextValue] - u[++prevValue] + v[++nextRow] - v[++previousRow]);
                p[currentRow] = 0;
            }
        }
        set_bnd(0, div);
        set_bnd(0, p);

        lin_solve(0, p, div, 1, 4 );
        var wScale = 0.5 * width;
        var hScale = 0.5 * height;
        for (var j = 1; j<= height; j++ ) {
            var prevPos = j * rowSize - 1;
            var currentPos = j * rowSize;
            var nextPos = j * rowSize + 1;
            var prevRow = (j - 1) * rowSize;
            var currentRow = j * rowSize;
            var nextRow = (j + 1) * rowSize;

            for (var i = 1; i<= width; i++) {
                u[++currentPos] -= wScale * (p[++nextPos] - p[++prevPos]);
                v[currentPos]   -= hScale * (p[++nextRow] - p[++prevRow]);
            }
        }
        set_bnd(1, u);
        set_bnd(2, v);
    }

    function dens_step(x, x0, u, v, dt)
    {
        addFields(x, x0, dt);
        diffuse(0, x0, x, dt );
        advect(0, x, x0, u, v, dt );
    }

    function vel_step(u, v, u0, v0, dt)
    {
        addFields(u, u0, dt );
        addFields(v, v0, dt );
        var temp = u0; u0 = u; u = temp;
        var temp = v0; v0 = v; v = temp;
        diffuse2(u,u0,v,v0, dt);
        project(u, v, u0, v0);
        var temp = u0; u0 = u; u = temp;
        var temp = v0; v0 = v; v = temp;
        advect(1, u, u0, u0, v0, dt);
        advect(2, v, v0, u0, v0, dt);
        project(u, v, u0, v0 );
    }
    var uiCallback = function(d,u,v) {};

    function Field(dens, u, v) {
        // Just exposing the fields here rather than using accessors is a measurable win during display (maybe 5%)
        // but makes the code ugly.
        this.setDensity = function(x, y, d) {
             dens[(x + 1) + (y + 1) * rowSize] = d;
        }
        this.getDensity = function(x, y) {
             return dens[(x + 1) + (y + 1) * rowSize];
        }
        this.setVelocity = function(x, y, xv, yv) {
             u[(x + 1) + (y + 1) * rowSize] = xv;
             v[(x + 1) + (y + 1) * rowSize] = yv;
        }
        this.getXVelocity = function(x, y) {
             return u[(x + 1) + (y + 1) * rowSize];
        }
        this.getYVelocity = function(x, y) {
             return v[(x + 1) + (y + 1) * rowSize];
        }
        this.width = function() { return width; }
        this.height = function() { return height; }
    }
    function queryUI(d, u, v)
    {
        for (var i = 0; i < size; i++)
            u[i] = v[i] = d[i] = 0.0;
        uiCallback(new Field(d, u, v));
    }

    this.update = function () {
        queryUI(dens_prev, u_prev, v_prev);
        vel_step(u, v, u_prev, v_prev, dt);
        dens_step(dens, dens_prev, u, v, dt);
        displayFunc(new Field(dens, u, v));
    }
    this.setDisplayFunction = function(func) {
        displayFunc = func;
    }

    this.iterations = function() { return iterations; }
    this.setIterations = function(iters) {
        if (iters > 0 && iters <= 100)
           iterations = iters;
    }
    this.setUICallback = function(callback) {
        uiCallback = callback;
    }
    var iterations = 10;
    var visc = 0.5;
    var dt = 0.1;
    var dens;
    var dens_prev;
    var u;
    var u_prev;
    var v;
    var v_prev;
    var width;
    var height;
    var rowSize;
    var size;
    var displayFunc;
    function reset()
    {
        rowSize = width + 2;
        size = (width+2)*(height+2);
        dens = new Array(size);
        dens_prev = new Array(size);
        u = new Array(size);
        u_prev = new Array(size);
        v = new Array(size);
        v_prev = new Array(size);
        for (var i = 0; i < size; i++)
            dens_prev[i] = u_prev[i] = v_prev[i] = dens[i] = u[i] = v[i] = 0;
    }
    this.reset = reset;
    this.getDens = function()
    {
        return dens;
    }
    this.setResolution = function (hRes, wRes)
    {
        var res = wRes * hRes;
        if (res > 0 && res < 1000000 && (wRes != width || hRes != height)) {
            width = wRes;
            height = hRes;
            reset();
            return true;
        }
        return false;
    }
    this.setResolution(64, 64);
}

setupNavierStokes()
;
runNavierStokes();
tearDownNavierStokes();

fib=function(y) { return (y >= 2 ? fib(y - 1) + fib(y - 2) : y); };
print(fib(32))
s = "magnus lidstrom";
for (var j = 0; j < 1000000; ++j) {
	for (var i = 0; s[i] !== (void 0); ++i);
}
x = '';
for (var i = 0; s[i] !== (void 0); ++i) x += s[i];
print(x);
