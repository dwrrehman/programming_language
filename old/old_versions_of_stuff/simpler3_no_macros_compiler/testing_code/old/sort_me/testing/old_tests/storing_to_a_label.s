(figuring out the syntax for storing to a label in this language...
1202504082.222311 dwrr
)

ct set debug 1

ct set system_exit 1
ct set system_write 2
ct set stdout 0

ct set length 0  (this is set later!)

sc system_write  stdout string_label length   0 0 0     

		 (^ a write system call! for printing some character to the screen. )

sc system_exit 101   0 0 0 0 0

		(an exit system call!)

halt



at string_label

	(your bytes here..?     we need to allocate some bytes here!!!)


ct st label 101
add length 1



ct halt





OH WAIT!! we can just put the code which STORESSS to this label, AFTERRRR the "at" lol. oh. 
easy peasy lol. nice. 

then of course, the runtime code will like, have to load the label somehow, into a register! niceeee





1202504082.223248
crappp

	but then  in order to actually have the length be picked up   even though its set is afterrr the system call, 

		we need to have the values actually stored into the instruction  AFTERRR execution lol. hmmmm interesting. 

			i guess we will do that then loll 


			i don't think theres any downside to it really lol... at least, i think?? hmmmmmm


			interestingg



	so yeah, we won't store the immediates into the instruction anymore, we will use references always, and just leave instructions as is, all the time, until all ct execution is fully complete. so yeah. hmmm interesting

		but, the thing is, this is kinddd of not very intuitive though.. hmmm idk... its interesting lol.. 

	i mean, i guess it kinda makes sense lol. this feature doesnt really come up like everrrr so i think its okay 



interesting







