print("this is a test message")

require "common"


function fNoArg()
	print("message from test")
end

function fTwoArg(a, b)
	print(a, b)
end

function fAdd(a, b)
	return a + b
end

function fReturnTwo(a, b)
	return a, b
end