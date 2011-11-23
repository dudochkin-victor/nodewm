PACKAGE = wmsigner
VERSION = 2.0.1
SHELL = /bin/sh

PREFIX = ${prefix}/usr/local
bindir = $(PREFIX)/bin
mandir = $(PREFIX)/man/man1
docdir = $(prefix)/usr/share/doc/wmsigner

# all dirs
DIRS = $(bindir) $(mandir) $(docdir)

# INSTALL scripts
INSTALL   	  = install -p --verbose
INSTALL_BIN     = $(INSTALL) -m 755
INSTALL_DIR     = $(INSTALL) -m 755 -d
INSTALL_DATA    = $(INSTALL) -m 644
INSTALL_DOC	= $(INSTALL) -m 644

all:   crypto.cpp md4.cpp rsalib1.cpp cmdbase.cpp signer.cpp wmsigner.cpp base64.cpp code64.cpp
	/usr/bin/g++ crypto.cpp md4.cpp rsalib1.cpp base64.cpp cmdbase.cpp signer.cpp wmsigner.cpp -o wmsigner
	/usr/bin/g++ code64.cpp base64.cpp -o code64 
	/bin/chmod g+x,o+x wmsigner code64 

test:   
	perl t/runtests
	rm -f test.b64 wmsigner.ini

install: wmsigner code64
	for dir in $(DIRS) ; do \
	$(INSTALL_DIR) $$dir ; \
	done
	$(INSTALL_BIN) wmsigner $(bindir)
	$(INSTALL_BIN) code64 $(bindir)
	$(INSTALL_DATA) wmsigner.1 $(mandir)
	$(INSTALL_DOC) INSTALL $(docdir)
	$(INSTALL_DOC) README $(docdir)
	$(INSTALL_DOC) README.rus $(docdir)
	$(INSTALL_DOC) ChangeLog $(docdir)

clean:
	rm -f wmsigner code64 test.b64 wmsigner.ini
