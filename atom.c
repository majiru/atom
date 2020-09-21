#include <u.h>
#include <libc.h>
#include <xml.h>

Xml *x;
Tzone *tz;
Elem *feed;

char*
unix2date(vlong abs)
{
	Tm tm;

	tmtime(&tm, abs, tz);
	return smprint("%τ", tmfmt(&tm, "YYYY-MM-DDThh:mm:ssZ"));
}

void
initmeta(char *title, char *link, char *author, char *id)
{
	Elem *e;

	feed = xmlelem(x, &x->root, nil, "feed");
	xmlattr(x, &feed->attrs, feed, "xmlns", "http://www.w3.org/2005/Atom");

	e = xmlelem(x, &feed->child, feed, "title");
	e->pcdata = title;

	e = xmlelem(x, &feed->child, feed, "link");
	xmlattr(x, &e->attrs, e, "href", link);

	e = xmlelem(x, &feed->child, feed, "updated");
	e->pcdata = unix2date(time(nil));

	e = xmlelem(x, &feed->child, feed, "author");
	e = xmlelem(x, &e->child, e, "name");
	e->pcdata = author;

	e = xmlelem(x, &feed->child, feed, "id");
	e->pcdata = id;
}

void
addentry(Dir *d, char *prefix)
{
	Elem *e, *entry;
	char *dot;

	entry = xmlelem(x, &feed->child, feed, "entry");

	e = xmlelem(x, &entry->child, entry, "title");
	e->pcdata = strdup(d->name);
	if((dot = strrchr(e->pcdata, '.')) != nil)
		*dot = '\0';

	e = xmlelem(x, &entry->child, entry, "link");
	xmlattr(x, &e->attrs, e, "href", smprint("%s/%s", prefix, d->name));

	e = xmlelem(x, &entry->child, entry, "id");
	e->pcdata = smprint("%ulld", d->qid.path);

	e = xmlelem(x, &entry->child, entry, "updated");
	e->pcdata = unix2date(d->mtime);
}

void
usage(void)
{
	fprint(2, "Usage: %s [-n name] [-i id] title link dir\n", argv0);
	exits("usage");
}

void
main(int argc, char *argv[])
{
	char *name, *id;
	Dir *files;
	long n, i;
	int fd;

	id = "9front atom";
	name = "glenda";

	ARGBEGIN {
	case 'n':
		name = EARGF(usage());
		break;
	case 'i':
		id = EARGF(usage());
		break;
	} ARGEND

	if(argc < 3)
		usage();

	tmfmtinstall();
	tz = tzload(nil);
	x = xmlnew(8192);

	fd = open(argv[2], OREAD);
	if(fd < 0)
		sysfatal("failed to read dir: %r");

	n = dirreadall(fd, &files);
	initmeta(argv[0], argv[1], name, id);
	for(i=0;i<n;i++)
		addentry(files+i, argv[1]);

	xmlprint(x, 1);
}
