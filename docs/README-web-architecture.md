## Web Architecture

### Overview

The web frontend is an Angular application with server-side rendering (SSR) powered by an Express server.
The app uses Angular's standalone APIs (no NgModules), router-based navigation, and client hydration for SSR.

The main flow is:

```text
Browser Request
↓
Express Server (SSR)
↓
AngularNodeAppEngine renders HTML
↓
Browser receives HTML
↓
Angular hydrates on the client
```

## Project Structure

```text
apps/web/
├── src/
│   ├── app/
│   │   ├── core/
│   │   │   ├── constants/
│   │   │   ├── interceptors/
│   │   │   ├── models/
│   │   │   ├── services/
│   │   │   ├── guards/
│   │   │   └── utils/
│   │   │
│   │   ├── shared/
│   │   │   ├── components/
│   │   │   │   ├── footer/
│   │   │   │   │   ├── footer.ts
│   │   │   │   │   ├── footer.html
│   │   │   │   │   └── footer.css
│   │   │   │   ├── navbar/
│   │   │   │   └── ui/
│   │   │   ├── directives/
│   │   │   └── pipes/
│   │   │
│   │   ├── features/
│   │   │   ├── auth/
│   │   │   │   ├── pages/
│   │   │   │   │   └── login-page/
│   │   │   │   ├── services/
│   │   │   │   └── models/
│   │   │   │
│   │   │   ├── public/
│   │   │   │   ├── home/
│   │   │   │   ├── about/
│   │   │   │   └── contact/
│   │   │   │
│   │   │   └── cms/
│   │   │       ├── dashboard/
│   │   │       ├── patients/
│   │   │       ├── appointments/
│   │   │       ├── users/
│   │   │       └── settings/
│   │   │
│   │   ├── layouts/
│   │   │   ├── public-layout/
│   │   │   └── cms-layout/
│   │   │
│   │   ├── app.ts
│   │   ├── app.html
│   │   ├── app.css
│   │   ├── app.routes.ts
│   │   ├── app.routes.server.ts
│   │   ├── app.config.ts
│   │   └── app.config.server.ts
│   │
│   ├── main.ts
│   ├── main.server.ts
│   ├── server.ts
│   ├── index.html
│   └── styles.css
│
├── public/
└── angular.json
```

The application is organized around standalone components and route configuration.
Server-side rendering is handled by `src/server.ts` with Angular SSR.

## Layer Responsibilities

### App Shell

`App` is the root component.

Responsibilities:

- Owns the app shell (`app.html`)
- Hosts the `RouterOutlet`
- Composes shared layout components (e.g., `Footer`)

### Components

Components encapsulate UI, template, and styles.

Example:

```text
Footer
├── footer.ts
├── footer.html
└── footer.css
```

Components should stay focused on view logic and presentation.

### Routes

Routing is defined in `app.routes.ts` for the client and `app.routes.server.ts` for SSR.

Responsibilities:

- Declare app navigation paths
- Control SSR render mode (currently `RenderMode.Prerender` for all routes)

### Application Config

Configuration is defined in `app.config.ts` and `app.config.server.ts`.

Responsibilities:

- Register providers (router, error listeners)
- Enable client hydration with event replay
- Merge server providers for SSR

### Server (SSR)

`src/server.ts` provides the Express host for SSR.

Responsibilities:

- Serve static assets from `dist/browser`
- Render Angular pages via `AngularNodeAppEngine`
- Expose a request handler for Angular CLI

This keeps web rendering and server concerns separated from UI components.

## Rendering and Hydration

The app supports SSR and client hydration:

- `main.server.ts` bootstraps Angular on the server
- `main.ts` bootstraps Angular in the browser
- `provideClientHydration` reuses the server-rendered HTML on the client

This improves initial load performance for slow devices while preserving SPA interactivity.

## Static Assets and Global Styles

- `public/` contains static assets copied during build
- `styles.css` contains global styles
- Component-level styles live next to their templates

## Planned Improvements

Planned enhancements include:

- Route configuration and page components
- Shared UI library and design system
- State management if the domain grows
- Error boundaries and user-facing error pages

## Technology Stack

| Technology  | Purpose               |
| ----------- | --------------------- |
| Angular     | Frontend framework    |
| Angular SSR | Server-side rendering |
| Express     | SSR HTTP server       |
| TypeScript  | Application language  |
| Angular CLI | Build and dev tooling |
