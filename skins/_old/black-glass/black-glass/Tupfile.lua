if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("black-glass.ASM", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "black-glass.skn")
