json = require('json')
dump = require('dump')
wifi = require('wifi')

while (1) do
    s = ''
    while (1) do
        c = io.read(1)
        if (c) then
            s = s..c
            if (c == '\n') then
                break
            end
        else
            sys.yield()
        end
    end

    f = load('return '..s)
    if (f) then
        print(f())
    else
        print('false')
    end
end