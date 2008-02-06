class Test
{
	function initialize(a, b, c)
	{
		var this.a = a;
		var this.b = b;
		var this.c = c;
	}

	property hash
	{
		getter() { return a.hash + b.hash + c.hash; }
	}

	function identify(r)
	{
		if(!(r instanceof Test)) return false;
		return a == r.a && b == r.b && c == r.c;
	}
};



var dic = new Dictionary();

dic["a"] = "string a";
dic["b"] = "string b";
dic["c"] = "string c";
dic[0] = "integer 0";
dic[1] = "integer 1";
dic[2] = "integer 2";
dic[-0.0] = "real -0.0";
dic[+0.0] = "real +0.0";
dic[void] = "void";
/*
comment out until we implement Null, Octet classes
dic[null] = "null";
dic[<% %>] = "octet empty";
dic[<% 01 %>] = "octet 1";
dic[<% 02 %>] = "octet 2";
*/
dic[true] = "true";
dic[false] = "false";
dic[Test.new(1, 2, 3)] = "Test 1,2,3";
dic[Test.new(0, 0, 0)] = "Test 0,0,0";
dic[Test.new(1, 1, 1)] = "Test 1,1,1";

assert(dic["a"] == "string a");
assert(dic["b"] == "string b");
assert(dic["c"] == "string c");
assert(dic[0] == "integer 0");
assert(dic[1] == "integer 1");
assert(dic[2] == "integer 2");
assert(dic[-0.0] == "real -0.0");
assert(dic[+0.0] == "real +0.0");
assert(dic[void] == "void");
/*
comment out until we implement Null, Octet classes
assert(dic[null] == "null");
assert(dic[<% %>] == "octet empty");
assert(dic[<% 01 %>] == "octet 1");
assert(dic[<% 02 %>] == "octet 2");
*/
assert(dic[true] == "true");
assert(dic[false] == "false");
assert(dic[Test.new(1, 2, 3)] == "Test 1,2,3");
assert(dic[Test.new(0, 0, 0)] == "Test 0,0,0");
assert(dic[Test.new(1, 1, 1)] == "Test 1,1,1");

"ok" //=> "ok"

