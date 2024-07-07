#!/usr/bin/perl
use strict;
use warnings;
use LWP::UserAgent;
use HTML::TreeBuilder;
use Data::Dumper;

# URL of the webpage to fetch
#my $url = 'https://perfmon-events.intel.com/ahybrid.html';
my $url = 'https://perfmon-events.intel.com/skylake.html';

# Create a user agent object
my $ua = LWP::UserAgent->new;

# Fetch the webpage
my $response = $ua->get($url);

# Check if the request was successful
unless ($response->is_success) {
    die "Failed to fetch the URL: ", $response->status_line;
}

sub get_element_by_path {
    my ($element, $path) = @_;
    my @segments = split '/', $path;

    # Remove empty first segment if path starts with '/'
    shift @segments if $segments[0] eq '';

    foreach my $segment (@segments) {
        if ($segment =~ /([a-zA-Z]+)\[(\d+)\]/) {
            my ($tag, $index) = ($1, $2);
            my @children = grep { $_->tag eq $tag } $element->content_list;
            $element = $children[$index - 1];
        } else {
            die "Invalid segment: $segment";
        }
    }

    return $element;
}

# Parse the HTML content
my $tree = HTML::TreeBuilder->new_from_content($response->decoded_content);

my $path = '/html/body/div[3]/div[4]/table/tbody/tr[3]/td[1]/span';
#my $x = get_element_by_path($tree, $path);

my $b = $tree->{'_body'}->{'_content'}->[3]->{'_content'}->[3]->{'_content'}->[0]->{'_content'};
for( my $id=1; $id<@{$b}; ++$id )
{
    #print "$id\n";
    my $x = $b->[$id]->{'_content'};
    my $name = $x->[0]->{'_content'}->[0]->{'_content'}->[0];
    my $d = $x->[2]->{'_content'}->[0];
    next if( !defined( $d ) );

    if( $d=~/^(IA32_FIXED_CTR[0-3]|MSR_UNC_PERF_FIXED_CTR) $/ )
    {
        next;
    }

    my $eventSel; if( $x->[2]->{'_content'}->[0]=~/EventSel=(.*?) / ) { $eventSel = $1; } else { die ".$x->[2]->{'_content'}->[0]."; };
    my $uMask; if( $x->[2]->{'_content'}->[0]=~/ UMask=(.*?) / ) { $uMask = $1; } else { die $x->[2]->{'_content'}->[0]; };
    my $cMask = "00"; if( $x->[2]->{'_content'}->[0]=~/ CMask=(.*?) / ) { $cMask = $1; }
    my $edgeDetect = "0"; if( $x->[2]->{'_content'}->[0]=~/ EdgeDetect=(.*?) / ) { $edgeDetect = $1; }
    my $invert = "0"; if( $x->[2]->{'_content'}->[0]=~/ Invert=(.*?) / ) { $invert = $1; }
    my $anyThread = "0"; if( $x->[2]->{'_content'}->[0]=~/ AnyThread=(.*?) / ) { $anyThread = $1; }
    my $pebs3f6 = "00"; if( $x->[2]->{'_content'}->[0]=~/ MSR_PEBS_LD_LAT_THRESHOLD\(3F6H\)=(.*?)$/ ) { $pebs3f6 = $1; };
    my $pebs3f7 = "00"; if( $x->[2]->{'_content'}->[0]=~/ MSR_PEBS_FRONTEND\(3F7H\)=(.*?)$/ ) { $pebs3f7 = $1; };
    my $offcore = "00"; if( $x->[2]->{'_content'}->[0]=~/ MSR_OFFCORE_RSPx\{1A6H,1A7H\}=(.*?)$/ ) { $offcore = $1; };
    
    my @y=split(" ", $x->[2]->{'_content'}->[0]);
    foreach my $y (@y)
    {
        die $y if( $y!~/(.*)=(.*)/ );
        #print "- $1 --- $2\n";
        if( $1 ne "MSR_PEBS_LD_LAT_THRESHOLD(3F6H)" and $1 ne "MSR_PEBS_FRONTEND(3F7H)" and $1 ne "UMask" and $1 ne "EventSel" and $1 ne "CMask" and $1 ne "EdgeDetect" and $1 ne "Invert"
            and $1 ne "MSR_OFFCORE_RSPx{1A6H,1A7H}" and $1 ne "AnyThread" )
        {
            die ".$1.";
        }
    }

    $eventSel =~ s/H$//;
    $uMask =~ s/H$//;
    $cMask =~ s/H$//;
    $pebs3f6 =~ s/ $//;
    $pebs3f6 =~ s/H$//;
    $pebs3f7 =~ s/ $//;
    $pebs3f7 =~ s/H$//;
    $offcore =~ s/H$//;

    printf("   , { 0x%s,                0x%s,  %s,          %s,         %s,      0x%s,  0x0%s,   0x0000%s,    0x0000000000%s, \"%s\" } // %s\n",
        $eventSel, $uMask, $edgeDetect, $anyThread, $invert, $cMask, $pebs3f6, $pebs3f7, $offcore, $name, $x->[1]->{'_content'}->[0] );
}

exit(0); my $x;
print ref($x), "\n";
if ( ref($x) eq "ARRAY")
{
    print "len:", 0+@$x;
    my $i=0;
    foreach my $s (@{$x})
    {
        print "\n$i ", $s->{'_tag'} if ref($s) ne "" and exists $s->{'_tag'};
        ++$i;
    }
    print "\n";
}
print "keys: ", join(",", keys %{$x}), "\n" if ref($x) eq "HTML::Element" or ref($x) eq "HTML::TreeBuilder";
print Dumper($x);
exit(0);

