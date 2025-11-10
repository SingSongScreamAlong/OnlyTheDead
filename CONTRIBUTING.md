# Contributing to Only The Dead

Thank you for your interest in contributing to **Only The Dead**! This project aims to create a historically accurate, emotionally powerful WWI survival game set at the Battle of Verdun.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [How Can I Contribute?](#how-can-i-contribute)
- [Development Setup](#development-setup)
- [Coding Standards](#coding-standards)
- [Commit Guidelines](#commit-guidelines)
- [Pull Request Process](#pull-request-process)
- [Historical Accuracy](#historical-accuracy)
- [Community](#community)

---

## Code of Conduct

This project honors the memory of 700,000 casualties. All contributions must:

- **Respectful**: Honor the subject matter (no glorification of war)
- **Accurate**: Maintain historical integrity
- **Collaborative**: Work constructively with others
- **Ethical**: No exploitative or insensitive content

---

## How Can I Contribute?

### Historical Research
- **Primary Sources**: Help find actual archival documents
- **German Perspective**: Expand German soldier accounts (currently underrepresented)
- **Verification**: Fact-check existing content against academic sources
- **Translation**: Translate French/German documents

### Game Design
- **Mission Design**: Create additional missions (30-35 total needed)
- **Balancing**: Test and tune survival mechanics
- **Narrative**: Write dialogue, letters, diary entries
- **Level Design**: Design specific locations and encounters

### Programming
- **Core Systems**: Implement C++ game systems
- **Blueprints**: Create UE5 Blueprint logic
- **Tools**: Build editor tools for designers
- **Optimization**: Performance profiling and fixes

### Art & Audio
- **3D Modeling**: Create historically accurate models
- **Texturing**: PBR materials for environment and characters
- **Animation**: Character and weapon animations
- **Sound Design**: Spatial audio, foley, voice acting
- **Music**: Period-appropriate composition

### Documentation
- **Technical Docs**: Improve setup guides
- **Tutorials**: Create video or written tutorials
- **Translation**: Localize documentation
- **Wiki**: Build community wiki

---

## Development Setup

See: [`docs/TECHNICAL_SETUP.md`](docs/TECHNICAL_SETUP.md)

**Prerequisites**:
- Unreal Engine 5.3+
- Visual Studio 2022 (Windows) or Clang (Linux/Mac)
- CMake 3.20+
- Git & Git LFS
- Python 3.10+

**Quick Start**:
```bash
git clone https://github.com/SingSongScreamAlong/OnlyTheDead.git
cd OnlyTheDead
git lfs pull
# Set UE5_ROOT environment variable
# Generate project files
# Open in Unreal Editor
```

---

## Coding Standards

### C++ Style

**Standard**: C++17
**Style Guide**: Unreal Engine Coding Standard + our additions

**Naming Conventions**:
- Classes: `PascalCase` (e.g., `USurvivalSystem`)
- Functions: `PascalCase` (e.g., `GetMeterValue()`)
- Variables: `camelCase` (e.g., `currentValue`)
- Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_HEALTH`)
- Unreal Prefixes:
  - `U` - UObject-derived classes
  - `A` - AActor-derived classes
  - `F` - Structs
  - `E` - Enums
  - `I` - Interfaces

**Example**:
```cpp
// Good
class ONLYTHEDEAD_API USurvivalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Survival")
    float GetMeterValue(ESurvivalMeterType MeterType) const;

private:
    float currentHunger = 100.0f;
    static constexpr float MAX_HUNGER = 100.0f;
};
```

**Comments**:
- All public functions must have /** Doxygen-style */ comments
- Complex logic needs inline comments explaining "why", not "what"
- Historical accuracy notes where applicable

**Example**:
```cpp
/**
 * Calculates thirst depletion rate based on activity and temperature.
 * Historical Note: Soldiers at Verdun often went 2-3 days without water
 * during sieges (e.g., Fort Vaux, June 1916).
 *
 * @param bIsActive Whether player is actively moving/fighting
 * @param Temperature Ambient temperature in Celsius
 * @return Depletion rate per hour (0.0-15.0)
 */
float CalculateThirstDepletionRate(bool bIsActive, float Temperature) const;
```

### Blueprint Standards

**Naming**: `BP_[Type]_[Name]`
Examples: `BP_Character_Jean`, `BP_System_Survival`

**Organization**:
- Use comments and colored sections
- Keep graphs readable (no spaghetti)
- Prefer functions over mega-graphs
- Use sub-graphs for complex logic

### Data Tables

**Format**: JSON (convert to UE5 DataTable)
**Naming**: `DT_[Category].json`
**Validation**: All JSON must be valid (test with `python -m json.tool`)

---

## Commit Guidelines

### Commit Message Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation
- `style`: Code style (formatting, no logic change)
- `refactor`: Code refactoring
- `perf`: Performance improvement
- `test`: Tests
- `chore`: Build process, tools

**Examples**:
```
feat(survival): Add hygiene disease system

Implements disease contraction based on hygiene levels.
Historical accuracy: Trench fever, dysentery, lice documented at Verdun.

Closes #42
```

```
fix(artillery): Correct 155mm blast radius calculation

Was using 20m, should be 25m per historical data.
Reference: French Army artillery manual 1916.
```

### Branch Naming

- `feature/descriptive-name` - New features
- `bugfix/issue-number-description` - Bug fixes
- `docs/what-changed` - Documentation
- `hotfix/critical-issue` - Critical fixes

**Examples**:
- `feature/fort-vaux-siege-mission`
- `bugfix/123-thirst-not-depleting`
- `docs/survival-system-guide`

---

## Pull Request Process

### Before Submitting

1. **Test Thoroughly**
   - All affected systems work
   - No new bugs introduced
   - Performance impact acceptable

2. **Code Quality**
   - Passes linting (if available)
   - No compiler warnings
   - Follows coding standards

3. **Documentation**
   - Update relevant docs
   - Add code comments
   - Update CHANGELOG.md

4. **Historical Accuracy**
   - Verify any historical claims
   - Cite sources in comments
   - Mark fictional content clearly

### PR Template

```markdown
## Description
Brief description of changes.

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-reviewed code
- [ ] Commented complex parts
- [ ] Updated documentation
- [ ] No new warnings
- [ ] Added tests (if applicable)
- [ ] Historical accuracy verified (if applicable)

## Testing
How was this tested?

## Screenshots
(if UI changes)

## Historical Sources
(if adding historical content)
```

### Review Process

1. **Automated Checks** (if CI/CD configured)
   - Build succeeds
   - Tests pass
   - Code style check

2. **Peer Review**
   - At least 1 approval required
   - Address all comments
   - Historical accuracy reviewed for content changes

3. **Merge**
   - Squash and merge (keep history clean)
   - Delete branch after merge

---

## Historical Accuracy

**This is a game, but it's based on real events that killed 700,000 people.**

### Guidelines

**Research**:
- Cite sources for all historical claims
- Use academic sources (not Wikipedia alone)
- Verify dates, locations, statistics
- Check facts against Research Anthology

**Tier System**:
- **Tier 1**: Verified historical fact (dates, events, statistics)
- **Tier 2**: Reconstructed from patterns (typical soldier experience)
- **Tier 3**: Game design (mechanics, made for gameplay)

**Label Content**:
```cpp
// TIER 1 - Historical Fact
// Fort Douaumont captured February 25, 1916 by Brandenburg Regiment
// Source: French official history, Service Historique de la Défense

// TIER 2 - Composite Reconstruction
// Typical daily ration based on French Army regulations 1916
// Character dialogue composite from soldier letter collections

// TIER 3 - Game Design
// Hunger meter depletion rate balanced for gameplay
```

**Prohibited**:
- Glorification of violence
- Disrespect for casualties
- Inaccurate stereotypes
- Fabricated "facts" presented as truth
- Gratuitous gore

**Encouraged**:
- Respectful portrayal of soldiers (both sides)
- Educational context
- Primary source references
- Shared humanity themes

---

## Community

### Communication Channels

**GitHub**: Issues and Pull Requests
**Discord**: (to be created) - Real-time chat
**Forum**: (to be created) - Long-form discussion
**Email**: [project email] - Private contact

### Getting Help

**For Technical Issues**:
- Check `docs/TECHNICAL_SETUP.md`
- Search existing GitHub issues
- Ask in Discord #dev-help
- Create new issue with template

**For Historical Questions**:
- Check `verdun_anthology/` docs
- Ask in Discord #history
- Consult with history team

**For Design Questions**:
- Check `ONLY_THE_DEAD_DEVELOPMENT_BOOK.md`
- Ask in Discord #design
- Discuss in design meetings

---

## Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md
- Credited in game credits
- Acknowledged in release notes

**Special Recognition For**:
- Major feature contributions
- Significant historical research
- Long-term maintainers

---

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (TBD - likely GPL v3 or similar for code, CC BY-SA for content).

---

## Questions?

- **Technical**: Open GitHub Issue with `question` label
- **Historical**: Email history team
- **General**: Ask in Discord

---

**Thank you for helping honor the memory of those who fought at Verdun.**

*"Ils ne passeront pas."*
