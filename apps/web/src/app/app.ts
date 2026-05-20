import { Component, signal } from '@angular/core';
import { RouterOutlet } from '@angular/router';
import { Footer } from './shared/components/footer/footer';
import { Nav } from './shared/components/nav/nav';


@Component({
  selector: 'app-root',
  imports: [RouterOutlet, Footer, Nav],
  templateUrl: './app.html',
  styleUrl: './app.css',
})
export class App {
  protected readonly title = signal('web');
}
