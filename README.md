# ASKAP Extract Spectra Case Study

Testing Extraction logic in ASKAP pipeline

## Dependencies

1. jsoncpp library [repo](https://github.com/open-source-parsers/jsoncpp/)

1. gsed (mac's built-in sed function does not work properly).
Can be installed using `brew install gnu-sed`

1. LOFAR common [repo](https://bitbucket.csiro.au/projects/askapsdp/repos/lofar-common/browse)

1. dvc: Data Version Control. To install: `brew install dvc`


<details>

<summary>Setting DVC shared cache ( WIP )</summary>

In this project, we are using shared cache instead of dvc remote, to share data files among all collaborators.
The cache is being shared through Google Shared Drive, which is synced with local machine using Google Drive Application.
To setup:

1. Get the Google Drive Application, sign in using TW google account. This will create a drive mount point in your mac.
Inside the `<Google Drive Mount Point>/Shared drives/E4R/Projects/CSIRO - yandasoft/extract-spectra-cst-remote`, you will see the required data files.

1. To setup that folder as the cache folder for your local project repository, run the following command inside your local project repo:

      ```sh
      dvc config --local cache.dir "<Google Drive Mount Point>/Shared drives/E4R/Projects/CSIRO - yandasoft/extract-spectra-cst-remote"
      ```

      Note: The path "Google Drive Mount Point" varies from user to user

1. Then run `dvc pull` to pull the actual files to local repo

</details>
