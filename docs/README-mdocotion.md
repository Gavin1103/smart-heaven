# Smart Heaven — City Sim

This repo holds the docs and code for our learning group’s smart city: Smart Heaven. We build a small, modular city on plywood tiles with laser‑engraved roads, then make it smart with sensors, lights, motors, and a simple backend/dashboard.

All project docs live in the `docs/` folder and are served using MkDocs + Material theme.

## Run the docs locally

1) Start once per clone to fetch theme/macros submodule:
```
git submodule update --init --recursive
```

2) Start the docs site (Docker required):
```
docker-compose up -d
```

3) Open the site: http://localhost:8000

Changes in `docs/` hot‑reload automatically.

## Contributing
- Add new features under `docs/features/` (copy `docs/features/_template.md`)
- Keep build rules in `docs/build-standards/` up to date
- Record important choices in `docs/logs/`
- Add images to `docs/assets/`

When you finish a sprint prototype, update its feature page with photos/gifs and a reset procedure.