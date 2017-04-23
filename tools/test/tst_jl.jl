#
# julia test
# @author tw
#

ccall((:tst_simple_call, "tst_jl.so"), Void, ())

ccall((:tst_string, "tst_jl.so"), Void, (Cstring,), "Test")

ccall((:tst_array, "tst_jl.so"), Void, (Array{Int64, 1},), [1,2,3,4,-5,-6,-7])
arr = ccall((:tst_array2, "tst_jl.so"), Array{Int64, 1}, ())
println("Array: ", arr)

