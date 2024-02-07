-- this is a comment!

--[[

	this is a multiline comment!

]]


local function stuff()
	print("a function was called!")
end

longstring = [[
	i am a very long string!
	this is a cool string lol 
]]

print(longstring)


print("hello world. this is an assembler.")

local hi = "hello!"


io.write("size of string ", #hi, "\n")



local number = 3

otherstuff = 1

print(number)
print(hi)
print(otherstuff)




stuff()

local a = 3
local b = 2

local answer = a + b

print(answer)
print(1 + 2)


-- local name = io.read()
-- print("hi "..name.."!")



-- if statements

local first = 5
local second = 6


if first == second then
	print("equal")
else 
	print("equal else")
end

if first ~= second then
	print("not equal")
else 
	print("not equal else")
end

if first < second then
	print("less")
else 
	print("less else")
end


if first >= second then
	print("g-eq")
else 
	print("g-eq else")
end








local bools = false
local bools2 = true
if bools and bools2 then print("true!") else print("false!") end




if bools and bools2 then print("true!") 
elseif not bools2 then print("the else if is true!") 
else print("false!") end



-- note:   always use local before var decls, so they have local scope like in c variables! 





local i = 0
while i < 100 do print(i); i = i + 1; end


local i = 0
while i < 100 do 
	print(i)
	i = i + 1
	if i == 8 then print("found eight!"); break end
end




local i = 0
repeat
	print(i)
	i = i + 1
until i >= 50


for i = 0, 10, 1 do
	print(i)
end





--  indicies    0  1  2  3  4  5   6   7         these are the correct ones, but lua is 1 based, which is stupid... anyways. 

primenumbers = {1, 2, 3, 5, 7, 11, 13, 17}

--                                 ^__________________ we want this one! 



for i, n in pairs(primenumbers) do 
	io.write("key is: ", i - 1, "   and value is: ", n, "\n")
end

io.write("                      the prime number at index \"6\" is:\t\t\t\t primenumbers[6] = ", primenumbers[6], "      ... but thats wrong\n")



io.write("to actually get the prime at index \"6\", you have to do:\t\t\t\t primenumbers[6 + 1] = ", primenumbers[6 + 1], "   ...which is right.\n")




--           				in the expression     array[6 + 1]             the  +1      
--
--                      is because we want to turn a zero based index, 6, 
--		     	    into a 1 based index. so we add 1. so yeah. 
--


print("this many primes: ")
print(#primenumbers)



-- now, i think we can get around this by doing this:




zerobased_numbers  = {[0] = 1, 2, 3, 5, 7, 11, 13, 17}

print("buttttt   this works!!!")
print(zerobased_numbers[6])
print("yay")


print("but this doesnt work at all... very cursed lololol")

for i, n in pairs(zerobased_numbers) do 
	io.write("key is: ", i, "   and value is: ", n, "\n")
end


print("instead, we will have to write this i think! but thats totally encouraged, and welcomed, because its very c like. yay. ")


local count = #zerobased_numbers

for i = 0, count, 1 do 
	io.write("key is: ", i, "   and value is: ", zerobased_numbers[i], "\n")
end

print("that works quite well, actually. nice. yay. cool pasta.")








-- heres initializing an empty array 



local array = {}
local length = 1000
for i = 0, length - 1, 1 do array[i] = 0 end

array[555] = 9813794


local function print_nats(array, count)
	for i = 0, count - 1, 1 do 
		if i % 8 == 0 then print("") end
		io.write("[", i, "]:", array[i], ", ") 
	end
	print("")
end

print_nats(array, length)








-- or, even better    than using that stupid for loop thing where you have to do -1 on the length, to stop it from using that index, 
--                   is to just make the loop yourself! using the while loop! that way you can't mess it up lololol. yay. lets use that. 



-- so i could rewrite everything by doing this:



local array2 = {}
local length2 = 1000

local i = 0
while i < length2 do 
	array2[i] = 0
	i = i + 1
end

local function print_nats2(array, count)
	local i = 0
	while i < count do 
		if i % 8 == 0 then print("") end
		io.write(i, ":", array[i], " ")
		i = i + 1
	end
	print("")
end


print_nats2(array2, length2)



io.write(string.format("this is the formatted length: %10d\n", length2))

print("so yeah, thats basically printf. basically.")







--[[

do
-- to jump to the local_label block
goto local_label
local no = 1
print("The number is : ", no)
-- within the scope of the local variable, so can not jump
::local_label::
no = no + 1
-- The scope of the local variable ends, so you can jump.
::ok::
end

]]



do
goto ok           -- was         goto local_label
local n = 0
print("The number is : ", n)
-- within the scope of the local variable, so can not jump
::local_label::
n = n + 1
-- The scope of the local variable ends, so you can jump.
::ok::
end



local n = 1092384

if n == 0 then goto ok end                                     -- branch statement.  NICE!!! 

print("the number is : ", n)

::ok::                                                    -- label attribution. 

print("this is the okay part. always executed.")









-- lets try to make a dowhile loop now using these!!! wow. 




local i = 0

::loop::
	print(i)
	i = i + 1
if i < 14 then goto loop end


--                this code above is abolsutely beautiful. i am so happy that lua allows this!!! 
--              wow, except for the not having i++, this language is actually usable, considering you can do 
--                          this control flow stuff. wow. nice. thats actually so cool lol. yay






-- now, lets do file io:


file = io.open("test.lua", "r")

if file == nil then 
	print("error: could not open file \"test.lua\"")
	os.exit(1) 
end 

file:seek("set", 0)

string = file:read("*a")
io.write("found this as the file contents: \"", string, "\", which is cool.\n")

file:close()


-- file writing is done using            file:write("hello world\n")



local n = 0
if n then print("this is cursed") else print("everything is fine lol..") end

local n = nil
if n then print("this is cursed") else print("everything is fine lol..") end


-- omg i hate that.   thats actively disgusting                         but whatever, i guess i can just say



local n = 0
if n ~= 0 then print("this is stupid") else print("this is also stupid...") end 






















local out = io.open("file.bin", "wb")
local str = string.char(72,101,108,108,111,10) -- "Hello\n"
out:write(str)
out:close()






local out = io.open("file.bin", "wb")
local t = {}
for i=0,255 do t[i+1] = i end
local str = string.char(unpack(t))
out:write(str)
out:close()










