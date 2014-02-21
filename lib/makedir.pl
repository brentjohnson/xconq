#! /usr/unsupported/bin/perl
foreach $file (@ARGV) {
    open(IMF,"<$file") || warn "Cannot open $file.\n";
    while (<IMF>) {
	if (/imf\s*\"([^\"]*)\"/) {
	    $imf = $1;
	    if ($where{$imf} && $where{$imf} ne $file) {
		warn "Family $imf present in files $where{$imf} and $file.\n";
	    } else {
		$where{$imf} = $file;
	    }
	}
    }
}

print "ImageFamilyName FileName\n";
foreach $imf (sort keys(%where)) {
    print "$imf $where{$imf}\n";
}
print ". .\n";
