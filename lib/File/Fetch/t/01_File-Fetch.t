BEGIN { chdir 't' if -d 't' };

use strict;
use lib '../lib';

use Test::More 'no_plan';

use Cwd             qw[cwd];
use File::Basename  qw[basename];
use Data::Dumper;

use_ok('File::Fetch');

### optionally set debugging ###
$File::Fetch::DEBUG = $File::Fetch::DEBUG   = 1 if $ARGV[0];
$IPC::Cmd::DEBUG    = $IPC::Cmd::DEBUG      = 1 if $ARGV[0];

unless( $ENV{PERL_CORE} ) {
    warn qq[

####################### NOTE ##############################

Some of these tests assume you are connected to the
internet. If you are not, or if certain protocols or hosts
are blocked and/or firewalled, these tests could fail due
to no fault of the module itself.

###########################################################

];

    sleep 3 unless $File::Fetch::DEBUG;
}

### show us the tools IPC::Cmd will use to run binary programs
if( $File::Fetch::DEBUG ) {
    ### stupid 'used only once' warnings ;(
    diag( "IPC::Run enabled: " . 
            $IPC::Cmd::USE_IPC_RUN || $IPC::Cmd::USE_IPC_RUN );
    diag( "IPC::Run available: " . IPC::Cmd->can_use_ipc_run );
    diag( "IPC::Run vesion: $IPC::Run::VERSION" );
    diag( "IPC::Open3 enabled: " . 
            $IPC::Cmd::USE_IPC_OPEN3 || $IPC::Cmd::USE_IPC_OPEN3 );
    diag( "IPC::Open3 available: " . IPC::Cmd->can_use_ipc_open3 );
    diag( "IPC::Open3 vesion: $IPC::Open3::VERSION" );
}

### _parse_uri tests
### these go on all platforms
my @map = (
    {   uri     => 'ftp://cpan.org/pub/mirror/index.txt',
        scheme  => 'ftp',
        host    => 'cpan.org',
        path    => '/pub/mirror/',
        file    => 'index.txt'
    },
    {	uri	    => 'rsync://cpan.pair.com/CPAN/MIRRORING.FROM',
        scheme	=> 'rsync',
        host	=> 'cpan.pair.com',
        path	=> '/CPAN/',
        file	=> 'MIRRORING.FROM',
    },
    {   uri     => 'http://localhost/tmp/index.txt',
        scheme  => 'http',
        host    => 'localhost',          # host is empty only on 'file://' 
        path    => '/tmp/',
        file    => 'index.txt',
    },  
    
    ### only test host part, the rest is OS dependant
    {   uri     => 'file://localhost/tmp/index.txt',
        host    => '',                  # host should be empty on 'file://'
    },        
);

### these only if we're not on win32/vms
push @map, (
    {   uri     => 'file:///usr/local/tmp/foo.txt',
        scheme  => 'file',
        host    => '',
        path    => '/usr/local/tmp/',
        file    => 'foo.txt',
    },
    {   uri     => 'file://hostname/tmp/foo.txt',
        scheme  => 'file',
        host    => 'hostname',
        path    => '/tmp/',
        file    => 'foo.txt',
    },    
) if not &File::Fetch::ON_WIN and not &File::Fetch::ON_VMS;

### these only on win32
push @map, (
    {   uri     => 'file:////hostname/share/tmp/foo.txt',
        scheme  => 'file',
        host    => 'hostname',
        share   => 'share',
        path    => '/tmp/',
        file    => 'foo.txt',
    },
    {   uri     => 'file:///D:/tmp/foo.txt',
        scheme  => 'file',
        host    => '',
        vol     => 'D:',
        path    => '/tmp/',
        file    => 'foo.txt',
    },    
    {   uri     => 'file:///D|/tmp/foo.txt',
        scheme  => 'file',
        host    => '',
        vol     => 'D:',
        path    => '/tmp/',
        file    => 'foo.txt',
    },    
) if &File::Fetch::ON_WIN;


### sanity tests
{   like( $File::Fetch::USER_AGENT, qr/$File::Fetch::VERSION/,
                                "User agent contains version" );
    like( $File::Fetch::FROM_EMAIL, qr/@/,
                                q[Email contains '@'] );
}                                

### parse uri tests ###
for my $entry (@map ) {
    my $uri = $entry->{'uri'};

    my $href = File::Fetch->_parse_uri( $uri );
    ok( $href,  "Able to parse uri '$uri'" );

    for my $key ( sort keys %$entry ) {
        is( $href->{$key}, $entry->{$key},
                "   '$key' ok ($entry->{$key}) for $uri");
    }
}

### File::Fetch->new tests ###
for my $entry (@map) {
    my $ff = File::Fetch->new( uri => $entry->{uri} );

    ok( $ff,                    "Object for uri '$entry->{uri}'" );
    isa_ok( $ff, "File::Fetch", "   Object" );

    for my $acc ( keys %$entry ) {
        is( $ff->$acc(), $entry->{$acc},
                                "   Accessor '$acc' ok ($entry->{$acc})" );
    }
}

### fetch() tests ###

### file:// tests ###
{
    my $prefix = &File::Fetch::ON_UNIX ? 'file://' : 'file:///';
    my $uri = $prefix . cwd() .'/'. basename($0);

    for (qw[lwp lftp file]) {
        _fetch_uri( file => $uri, $_ );
    }
}

### ftp:// tests ###
{   my $uri = 'ftp://ftp.funet.fi/pub/CPAN/index.html';
    for (qw[lwp netftp wget curl lftp ncftp]) {

        ### STUPID STUPID warnings ###
        next if $_ eq 'ncftp' and $File::Fetch::FTP_PASSIVE
                              and $File::Fetch::FTP_PASSIVE;

        _fetch_uri( ftp => $uri, $_ );
    }
}

### http:// tests ###
{   for my $uri ( 'http://www.cpan.org/index.html',
                  'http://www.cpan.org/index.html?q=1',
                  'http://www.cpan.org/index.html?q=1&y=2',
    ) {
        for (qw[lwp wget curl lftp lynx]) {
            _fetch_uri( http => $uri, $_ );
        }
    }
}

### rsync:// tests ###
{   my $uri = 'rsync://cpan.pair.com/CPAN/MIRRORING.FROM';

    for (qw[rsync]) {
        _fetch_uri( rsync => $uri, $_ );
    }
}

sub _fetch_uri {
    my $type    = shift;
    my $uri     = shift;
    my $method  = shift or return;

    SKIP: {
        skip "'$method' fetching tests disabled under perl core", 4
                if $ENV{PERL_CORE};
    
        ### stupid warnings ###
        $File::Fetch::METHODS =
        $File::Fetch::METHODS = { $type => [$method] };
    
        my $ff  = File::Fetch->new( uri => $uri );
    
        ok( $ff,                "FF object for $uri (fetch with $method)" );
    
        my $file = $ff->fetch( to => 'tmp' );
    
        SKIP: {
            skip "You do not have '$method' installed/available", 3
                if $File::Fetch::METHOD_FAIL->{$method} &&
                   $File::Fetch::METHOD_FAIL->{$method};
                
            ### if the file wasn't fetched, it may be a network/firewall issue                
            skip "Fetch failed; no network connectivity for '$type'?", 3 
                unless $file;
                
            ok( $file,          "   File ($file) fetched with $method ($uri)" );
            ok( $file && -s $file,   
                                "   File has size" );
            is( $file && basename($file), $ff->output_file,
                                "   File has expected name" );
    
            unlink $file;
        }
    }
}








