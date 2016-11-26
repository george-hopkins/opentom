#!/bin/bash

po=po/$1.po.in
if [ ! -f $po ] ; then
	echo $po not found
	echo usage:
	echo $0 lang_code files
	echo
	exit 1
fi

shift

for i in $* ; do
	
	echo $i

	if [ ! -f $i.int ] ; then
		cp $i $i.int
	fi

	if [ ! -f $i.int ] ; then
		echo $i.int not found
		exit 1
	fi

	awk  -v po=$po 'BEGIN{
		while (getline < po ==1)
		{
			if($1=="msgid_plural")
			{
				sip=substr($0,14)
				gsub(/\|/,"\\|",sip)
				gsub(/\$/,"\\$",sip)
				gsub(/\(/,"\\(",sip)
			}	
			if($1=="msgid")
			{
				si=substr($0,7)
				gsub(/\|/,"\\|",si)
				gsub(/\$/,"\\$",si)
				gsub(/\(/,"\\(",si)
			}	
			if($1=="msgstr")
			{
				so=substr($0,8)
				if(so=="\"\"")continue
				if(si=="\"\"")continue
				t["_[(].?" si ")" ] = "(" so ")"
			}	
			if($1=="msgstr[0]")
			{
				so=substr($0,11)
				if(so=="\"\"")continue
				if(si=="\"\"")continue
				t["_[(].?" si ")" ] = "(" so ")"
			}	
			if($1=="msgstr[1]")
			{
				so=substr($0,11)
				if(so=="\"\"")continue
				if(sip=="\"\"")continue
				t["_[(].?" sip ")" ] = "(" so ")"
			}	
	
		}
#		for( i in t)
#		{
#			print i,t[i]
#		}
	}
	{
	match($0,"navit_nls_ngettext")
	if(RSTART)
	{
		o=substr($0,1,RSTART+RLENGTH)
		ac=0
		for(i=RSTART+RLENGTH+1;;i++)
		{
			c=substr($0,i,1)
			if(c=="")break
			if(c=="\"")
			{
				ac++
				if(ac==1||ac==3) o=o "_(\"" 	
				if(ac==2||ac==4) o=o "\")" 
			}
			else o=o c
		}
		$0=o
	}
		if($0~/_\(/)
		{
			for( i in t)
			{
				gsub(i,t[i])
			}
		}
		print $0
	}' $i.int > $i

done

