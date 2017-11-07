
function test(param)

	local fInner = function(p)
		local a = 1
		local b = 2
		print(a, b)
		Debug()
	end

	fInner()
end

test(123)