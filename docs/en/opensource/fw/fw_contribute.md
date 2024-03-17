---
comments: true
---

# How to contribute to the firmware repository?

![contributing to repositories by Dall-e](img/github_contribute.png ){ align=right width=350 }

You can contribute to the repo by:

- Submitting a bug report in github
- Suggesting an improvement
- Submitting code (doing a pull request to the develop branch)

## Submitting a bug

#### Before Submitting a Bug Report

A good bug report shouldn't leave others needing to chase you up for more information. Therefore, we ask you to investigate carefully, collect information and describe the issue in detail in your report. Please complete the following steps in advance to help us fix any potential bug as fast as possible.

- Make sure that you are** using the latest version**.
- **Determine if your bug is really a bug** and not an error on your side e.g. using incompatible environment components/versions (Make sure that you have read the [documentation](https://deepdeck.co/en/opensource/fw/firmware/).
- To **see if other users have experienced** (and potentially already solved) the same issue you are having, check if there is not already a bug report existing for your bug or error in the [bug tracker](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/issues?q=label%3Abug).
- Also make sure to search the internet (including Stack Overflow) to see if users outside of the GitHub community have discussed the issue (specially if it has to do with ESP32 IDF, rather than the functionality itself).
- Collect information about the bug:
    - Stack trace (Traceback)
    - OS, Platform and Version (Windows, Linux, macOS, x86, ARM)
    - Version of the interpreter, compiler, SDK, runtime environment, package manager, depending on what seems relevant.
    - Possibly your input and the output
    - Can you reliably reproduce the issue? And can you also reproduce it with older versions?


#### How Do I Submit a Good Bug Report?

> You must never report security related issues, vulnerabilities or bugs including sensitive information to the issue tracker, or elsewhere in public. Instead sensitive bugs must be sent by email to deepdeck@dsd.dev.

We use GitHub issues to track bugs and errors. If you run into an issue with the project:

- Open an [Issue](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/issues/new). (Since we can't be sure at this point whether it is a bug or not, we ask you not to talk about a bug yet and not to label the issue.)
- Explain the behavior you would expect and the actual behavior.
- Please provide as much context as possible and describe the *reproduction steps* that someone else can follow to recreate the issue on their own. This usually includes your code. For good bug reports you should isolate the problem and create a reduced test case.
- Provide the information you collected in the previous section.

Once it's filed:

- The project team will label the issue accordingly.
- A team member will try to reproduce the issue with your provided steps. If there are no reproduction steps or no obvious way to reproduce the issue, the team will ask you for those steps and mark the issue as `needs-repro`. Bugs with the `needs-repro` tag will not be addressed until they are reproduced.
- If the team is able to reproduce the issue, it will be marked `needs-fix`, as well as possibly other tags (such as `critical`), and the issue will be left to be implemented by someone.


## Suggesting Enhancements

This section guides you through submitting an enhancement suggestion for DeepDeck Ahuyama Firmware, **including completely new features and minor improvements to existing functionality**. Following these guidelines will help maintainers and the community to understand your suggestion and find related suggestions.

### Before Submitting an Enhancement

- Make sure that you are using the latest version.
- Read the [documentation](/en/QuickStartGuide/qsg/) carefully and find out if the functionality is already covered, maybe by an individual configuration.
- Perform a [search](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/issues) to see if the enhancement has already been suggested. If it has, add a comment to the existing issue instead of opening a new one.
- Find out whether your idea fits with the scope and aims of the project. It's up to you to make a strong case to convince the project's developers of the merits of this feature. Keep in mind that we want features that will be useful to the majority of our users and not just a small subset. If you're just targeting a minority of users, consider writing an add-on/plugin library.


### How Do I Submit a Good Enhancement Suggestion?

Enhancement suggestions are tracked as [GitHub issues](https://github.com/DeepSea-Developments/DeepDeck.Ahuyama.fw/issues).

- Use a **clear and descriptive title** for the issue to identify the suggestion.
- Provide a **step-by-step description of the suggested enhancement** in as many details as possible.
- **Describe the current behavior** and **explain which behavior you expected to see instead** and why. At this point you can also tell which alternatives do not work for you.
- You may want to **include screenshots and animated GIFs** which help you demonstrate the steps or point out the part which the suggestion is related to. You can use [this tool](https://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
- **Explain why this enhancement would be useful** to most DeepDeck Ahuyama Firmware users. You may also want to point out the other projects that solved it better and which could serve as inspiration.


## Code Contribution

First follow the guide [to set up everything in VScode](/en/opensource/fw/vscode-setup/).

Fork the repo and follow the [gitflow structure](/en/opensource/fw/repository/) to know where to work (hint: develop branch).

Before doing a pull request, make sure that:
- The code compile
- You are using the latest version of the code in the develop branch
- You did not break other stuff. We will work on a testing guide, but for now, it's a manual process.

In the pull request, please explain if the code is for solving a bug, or doing an enhancement. Explain what is the outcome and some information of the testing we have to make to your new feature to make sure it works.

Any doubt you can join our discord community and ask, or email deepdeck@dsd.dev.