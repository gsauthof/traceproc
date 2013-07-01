file example

set print pretty on
set height 0

b sqlcxt
commands 1
set $x=$rdx
p *(struct sqlexd*) $rdx
c
end

b *(sqlcxt+732)
commands 2
p *(struct sqlexd*) $x
c
end

set logging file trace
set logging on

