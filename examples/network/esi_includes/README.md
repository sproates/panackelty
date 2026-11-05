# Two pages, one banner

Run from the repository root:

```sh
./bin/panack run --project examples/network/esi_includes/panack.json 8082
```

Open [Home](http://127.0.0.1:8082/) and [About](http://127.0.0.1:8082/about).
Edit [the banner](public/fragments/banner.html), then refresh either page.
The server reads the page template and fragment on each request. Ctrl-C stops it.

Both templates contain the same directive:

```html
<esi:include src="/fragments/banner.html"/>
```

The route handler expands that markup before sending the HTML. This small
subset of Edge Side Includes accepts one directive in exactly the form above
per page. The leading slash identifies the example's public directory.
A confined filesystem read limits each input to 4 KiB. Missing or inaccessible
fragments, nested directives and unsupported ESI elements produce HTTP 500.
See [the handler](src/main.panack) for the assembly step.
