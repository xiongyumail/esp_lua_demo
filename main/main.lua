json = require('json')
dump = require('dump')
wifi = require('wifi')

while (1) do
    s = ''
    while (1) do
        c = io.read("*L")
        if (c) then
            s = s..c
            if (string.find(s, "\n")) then
                break;
            else
                sys.yield()  
            end
        else
            sys.yield() 
        end
    end

    if pcall(load(s)) then
        print('true')
    else
        print('false')
    end
end