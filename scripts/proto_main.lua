UpdateRootProto("luatest.proto")

local tTest = {
	iFirst = 123,
	sSecond = "这是一个脚本字符串"
}

local sPb = PBEncode("tTestProto", tTest)

local tResult = PBDecode("tTestProto", sPb)

print("结果是:")
for k,v in pairs(tResult) do
	print(k,v)
end