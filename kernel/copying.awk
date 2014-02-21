BEGIN	{
	  FS="\"";
	  print "/* ==> Do not modify this file!!  It is created automatically";
	  print "   by copying.awk.  Modify copying.awk instead.  <== */";
	  print ""
	  print "#include \"conq.h\""
	  print ""
	  print "void";
	  print "describe_copyright(int arg, char *key, TextBuffer *buf)";
	  print "{";
	}
NR == 1,/^[ 	]*NO WARRANTY[ 	]*$/	{
	  if ($0 ~ //)
	    {
	      printf "    tbcatline(buf, \"\");\n";
	    }
	  else if ($0 !~ /^[ 	]*NO WARRANTY[ 	]*$/) 
	    {
	      printf "    tbcatline(buf, \"";
	      for (i = 1; i < NF; i++)
		printf "%s\\\"", $i;
	      printf "%s\");\n", $NF;
	    }
	}
/^[	 ]*NO WARRANTY[ 	]*$/	{
	  print "}";
	  print "";
	  print "void";
	  print "describe_warranty(int arg, char *key, TextBuffer *buf)";
	  print "{";
	}
/^[ 	]*NO WARRANTY[ 	]*$/, /^[ 	]*END OF TERMS AND CONDITIONS[ 	]*$/{  
	  if (! ($0 ~ /^[ 	]*END OF TERMS AND CONDITIONS[ 	]*$/)) 
	    {
	      printf "    tbcatline(buf, \"";
	      for (i = 1; i < NF; i++)
		printf "%s\\\"", $i;
	      printf "%s\");\n", $NF;
	    }
	}
END	{
	  print "}";
	}
