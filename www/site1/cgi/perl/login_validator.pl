#!/usr/bin/env perl
use strict;
use warnings;

sub url_decode {
    my ($str) = @_;

    $str =~ tr/+/ /;
    $str =~ s/%([0-9A-Fa-f]{2})/chr(hex($1))/eg;

    return $str;
}

sub read_stdin_body {
    my $content_length = $ENV{'CONTENT_LENGTH'} || 0;
    my $body = '';

    if ($content_length > 0) {
        read(STDIN, $body, $content_length);
    } else {
        while (my $line = <STDIN>) {
            $body .= $line;
        }
    }

    return $body;
}

sub extract_login {
    my ($body) = @_;
    my @pairs = split /&/, $body;

    foreach my $pair (@pairs) {
        my ($key, $value) = split /=/, $pair, 2;
        $key   = defined($key)   ? url_decode($key)   : '';
        $value = defined($value) ? url_decode($value) : '';

        if ($key eq 'login' || $key eq 'USER_NAME') {
            $value =~ s/[\r\n]+\z//;
            return $value;
        }
    }

    return '';
}

sub is_valid_login {
    my ($login) = @_;

    return 0 if ($login eq '');

    return ($login =~ /^[A-Z][A-Za-z0-9]*$/) ? 1 : 0;
}

sub read_file_content {
    my ($filename) = @_;
    my $fh;

    if (!open($fh, '<', $filename)) {
        return (
            "Status: 500 Internal Server Error\r\n" .
            "Content-Type: text/html\r\n" .
            "\r\n" .
            "<!DOCTYPE html><html><body><h1>500 Internal Server Error</h1><p>Cannot open file.</p></body></html>"
        );
    }

    local $/;
    my $content = <$fh>;
    close($fh);

    return $content;
}

sub send_response {
    my ($content, $login, $is_valid) = @_;

    my $length = length($content);

    print "Content-Type: text/html\r\n";
    print "Content-Length: $length\r\n";

    if ($is_valid) {
        print "New-Login: $login\r\n";
    }

    print "\r\n";
    print $content;
}

sub main {
    my $body = read_stdin_body();
    my $login = extract_login($body);
    my $valid = is_valid_login($login);

    my $filename = $valid
        ? './www/site1/cgi/perl/valid_login.html'
        : './www/site1/cgi/perl/invalid_login.html';

    my $content = read_file_content($filename);

    if ($content =~ /^Status: 500 Internal Server Error\r\n/) {
        print $content;
        return;
    }

    send_response($content, $login, $valid);
}

main();