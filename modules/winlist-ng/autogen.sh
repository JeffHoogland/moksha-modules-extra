#!/bin/sh

rm -rf autom4te.cache
rm -f aclocal.m4 ltmain.sh

touch README

echo "Running autopoint..." ; autopoint -f || :
echo "Running aclocal..." ; aclocal -I m4 $ACLOCAL_FLAGS || exit 1
echo "Running autoheader..." ; autoheader || exit 1
echo "Running autoconf..." ; autoconf || exit 1
echo "Running libtoolize..." ; (libtoolize --copy --automake || glibtoolize --automake) || exit 1
echo "Running automake..." ; automake --add-missing --copy --gnu || exit 1
echo "Generating gettext ng.pot.template";\
xgettext \
-n \
-C \
-d ng \
-p po \
--copyright-holder="Enlightenment development team" \
--foreign-user \
--msgid-bugs-address="enlightenment-devel@lists.sourceforge.net" \
-kD_ \
--from-code=UTF-8 \
-o ng.pot \
`find . -name "*.[ch]" -print` || exit 1

if [ -z "$NOCONFIGURE" ]; then
	./configure "$@"
fi
