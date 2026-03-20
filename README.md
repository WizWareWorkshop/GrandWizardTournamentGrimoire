# 📚 Grimoire Plugin 📚

## 📖 What this repository is

This repository contains the Grimoire Plugin, the spell-authoring and spell-logic foundation of **Grand Wizard Tournament**.

Grand Wizard Tournament is a fantasy action game built in Unreal Engine 5 with a strong focus on spellcrafting, magical system design, and spells as authored logic rather than static abilities. The purpose of this repository is to make that direction understandable through the plugin that sits at the heart of it.

Although this is a plugin repository and not the full game repository, it is meant to explain the larger vision as well. The Grimoire is not a side feature disconnected from the game. It is one of the core ideas behind the project. This repository therefore serves both as a technical home for the plugin and as a public-facing explanation of how spellcraft is intended to work inside Grand Wizard Tournament.

## 🌟 The larger vision behind the project

The long-term vision of Grand Wizard Tournament is that spells should not feel like fixed entries in a menu. A spell should feel like something built, understood, and shaped through magical logic.

The fantasy behind the system is not simply that a wizard casts power. The fantasy is that a wizard understands power.

That is the role of the Grimoire.

The Grimoire is intended to become the place where spells are authored as structured magical logic. It is where a spell gains shape, flow, conditions, internal rules, and meaning. Instead of thinking of a spell only as an effect, the system treats a spell as something that can be composed, inspected, validated, and ultimately translated into gameplay.

This plugin exists because that idea needs a real technical foundation.

## 🪄 What the Grimoire is meant to become

The current graph editor is planned as the foundation of a larger Grimoire system.

At first glance, that may sound like a standard node editor. But the intended goal is much broader than building a visual scripting surface. The graph editor is not meant to remain a loose prototype where nodes are connected for experimentation only. It is meant to evolve into a real Grimoire pipeline in which authored spell graphs become formal spell definitions that can be checked, compiled, and executed in the game.

That means the graph editor is only the first visible layer of a deeper structure.

Over time, the intended flow is:

1. a spell is authored visually,
2. the authored structure is validated,
3. the validated spell is compiled into runtime data,
4. the runtime data is executed through game systems,
5. and the result becomes a real gameplay spell.

In that sense, the Grimoire is where magic is designed, but also where magical design becomes runnable logic.

## 🧱 Core system direction

The intended architecture is built around several distinct responsibilities.

### Authoring

The authoring layer is the graph editor.

This is where spells are created visually through structure, flow, conditions, values, triggers, branches, and effects. The purpose of authoring is not only convenience, but clarity. A spell should have a shape that can be read and reasoned about.

### Validation

A spell graph should not go straight from idea to runtime.

Before execution, the system is intended to validate authored graphs and detect problems such as broken links, invalid structure, missing requirements, illegal combinations, or other authoring mistakes. Validation is important because the system should know whether a spell is coherent before the game tries to run it.

### Compilation

The compiler layer is what transforms authored spell structure into runtime-ready data.

This is one of the most important ideas in the whole system. The graph shown in the editor is not meant to be treated as the runtime spell itself. Instead, the graph becomes input to a compiler pipeline that extracts, transforms, and prepares spell information for actual gameplay execution.

That separation matters because it makes the system more maintainable, more testable, and more scalable over time.

### Runtime

At runtime, the compiled spell is intended to become actual game behavior.

The long-term runtime direction is grounded in Unreal Engine systems and uses Gameplay Ability System as the basis for things such as cast handling, authority, costs, cooldowns, and execution flow. The Grimoire therefore does not stop at authoring. It is meant to feed directly into the real gameplay layer.

### Integration and reaction

Spells do not exist in isolation.

They must interact with players, targets, combat state, conditions, feedback, and the larger game world. That is why the Grimoire is not only a graph storage tool. It is meant to become part of a wider gameplay architecture in which authored magical logic has visible consequences in the game.

## 🔍 Why this matters for Grand Wizard Tournament

Grand Wizard Tournament is not built around the idea that “magic is just another weapon.”

The project aims to give spellcraft its own identity. A powerful spell should not only be visually impressive or numerically strong. It should also feel authored, internally coherent, and understandable. The design of the spell system should support the fantasy that creating magic is itself a meaningful act.

That is why the Grimoire matters so much. It allows the game’s central fantasy to become systemic instead of decorative.

A spell is no longer only a predefined content entry.  
A spell becomes a constructed piece of magical logic.

## 🧠 Design philosophy

Several principles define the direction of this plugin.

### Spells should have structure

A spell should have an internal form that can be understood, not just an output effect.

### Spells should be readable

If a spell is authored through a graph, that graph should help explain what the spell does and why it behaves that way.

### Rules should exist before runtime

Broken magical logic should be identified before the game tries to execute it.

### Authoring and runtime should not be the same thing

The editor representation and the gameplay representation should be related, but not collapsed into one layer.

### Curation matters

This is not intended to become an unrestricted repository where every public contribution automatically becomes official game content. The project is curated, and the official game direction remains selective.

## 🚧 Current development status

This plugin is under active architectural development.

Earlier prototype work explored the broad idea of node-based spell creation, but the long-term plan is to rebuild the system on clearer and stronger boundaries. The goal is not to continue stacking major logic onto contradictory experimental foundations. The goal is to create a proper spell-authoring and spell-compilation architecture that can support the real game over time.

The current direction includes:
- a custom graph-based spell-authoring workflow,
- stricter system boundaries,
- validation as a real layer,
- compilation as a real layer,
- runtime execution separated from editor representation,
- and gameplay integration grounded in the game’s broader technical framework.

## 🤝 Why this repository is public

This repository is public for several reasons.

First, the Grimoire is one of the most interesting parts of the project, and it benefits from being visible and understandable.

Second, the repository provides a place where developers, designers, and interested players can study the architecture and understand the direction of the spell system.

Third, it creates a structured public space for spell ideas and discussion. People may eventually propose spell concepts, logic structures, and design ideas for review.

However, this public visibility should not be mistaken for unrestricted reuse or automatic inclusion. This repository is meant to support learning, discussion, visibility, and curated feedback. It is not intended to surrender ownership of the plugin, the game, or the project direction.

## ✍️ Spell ideas and curated review

One of the goals of making the Grimoire visible is to make spell design discussable.

The long-term idea is that people should be able to look at the system, understand how spells are meant to work, and propose their own spell concepts in a structured way. These ideas may include spell flow, conditions, variables, effect chains, balance logic, and other forms of magical design.

That does **not** mean that submitted ideas automatically become part of the game.

Grand Wizard Tournament is a curated project. Spell proposals may be reviewed, discussed, refined, rejected, or kept only as inspiration. Official inclusion remains entirely under project control.

## 📜 Rights and repository status

This repository is source-visible, but unless specific material is clearly marked otherwise, it is **not** open-source software.

The plugin is published so people can study it, follow its development, understand the spell system, and participate in discussion where invited. Public visibility does not mean general permission to reuse, redistribute, commercialize, port, or incorporate the repository’s code or other contents into other works.

For exact terms, see `LICENSE.md`.

## 🧭 Repository purpose in one sentence

This repository exists to show how the graph editor is intended to grow into a true Grimoire: a system where spells are authored as magical logic, validated as structured designs, and compiled into real runtime spell behavior for Grand Wizard Tournament.
