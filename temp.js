function testFunction() {
  var x = 0;
  var scope = {x: 1};

  with (scope) {
    x = (delete scope.x, 2);
  }

  print(scope.x);
  print(x);
}
testFunction();

